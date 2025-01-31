#include "uchen/tboard/tboard_file.h"

#include <climits>
#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "absl/log/log.h"

#include "crc32c/crc32c.h"
#include "proto/event.pb.h"
#include "proto/summary.pb.h"
#include "src/uchen/tboard/tboard_file.h"

namespace {

// MaskDelta is a fixed arbitrary number that is used to mask the CRC32
// checksum. expected from TensorBoard.
constexpr uint32_t kMaskDelta = 0xa282ead8ul;

}

namespace uchen::tboard {

absl::StatusOr<TBoardFile> TBoardFile::Open(std::string_view logs_directory,
                                            std::string_view log_name) {
  if (logs_directory.empty()) {
    return absl::InvalidArgumentError("Log file name must be provided");
  }

  std::string timestamp =
      absl::FormatTime("%Y%m%d-%H%M%S", absl::Now(), absl::UTCTimeZone());
  std::filesystem::path events_directory =
      std::filesystem::path(logs_directory) / std::filesystem::path(timestamp);

  if (!std::filesystem::exists(events_directory)) {
    if (!std::filesystem::create_directories(events_directory)) {
      return absl::InternalError("Failed to create logs directory");
    }
  }

  std::string hostname = "localhost";
  char hostname_buffer[HOST_NAME_MAX + 1];
  if (gethostname(hostname_buffer, sizeof(hostname_buffer)) == 0) {
    hostname = hostname_buffer;
  }

  std::string time_in_seconds = std::to_string(time(nullptr));

  std::string filename = absl::StrFormat("%s.out.tfevents.%s.%s", log_name,
                                         time_in_seconds, hostname);
  std::filesystem::path file_path =
      events_directory / std::filesystem::path(filename);
  std::ofstream file(file_path, std::ios::binary);

  if (!file.is_open()) {
    return absl::InternalError("Failed to open file");
  }
  return TBoardFile(std::move(file));
}

void TBoardFile::AddScalar(const std::string& tag, double value,
                           uint32_t step) {
  LOG(INFO) << "Adding scalar: " << tag << ": " << value << ", Step: " << value;

  tensorflow::Summary summary;

  tensorflow::Summary::Value* summary_value = summary.add_value();
  summary_value->set_tag(tag);
  summary_value->set_simple_value(value);

  AddEvent(summary, step);
}

void TBoardFile::AddImage(const std::string tag, const std::string& image,
                          uint32_t step, uint32_t width, uint32_t height,
                          uint32_t channel, const std::string& image_name,
                          const std::string& image_description) {
  auto encoded_image = EncodeImage(image);
  if (!encoded_image.ok()) {
    LOG(ERROR) << "Failed to Add image \"" << image_name
               << "\": " << encoded_image.status();
    return;
  }

  LOG(INFO) << "Recording image: " << tag;

  tensorflow::SummaryMetadata metadata;
  if (!image_name.empty()) {
    metadata.set_display_name(image_name);
  } else {
    metadata.set_display_name(tag);
  }
  metadata.set_summary_description(image_description);

  tensorflow::Summary::Image image_summary;
  image_summary.set_width(width);
  image_summary.set_height(height);
  image_summary.set_colorspace(channel);
  image_summary.set_encoded_image_string(encoded_image.value());

  tensorflow::Summary summary;
  tensorflow::Summary::Value* summary_value = summary.add_value();
  summary_value->set_tag(tag);
  summary_value->mutable_image()->Swap(&image_summary);
  summary_value->mutable_metadata()->Swap(&metadata);

  AddEvent(summary, step);
}

void TBoardFile::AddEvent(tensorflow::Summary& summary, uint32_t step) {
  tensorflow::Event event;
  event.set_wall_time(static_cast<double>(time(nullptr)));
  event.set_step(step);
  event.mutable_summary()->Swap(&summary);

  std::string buffer;
  event.SerializeToString(&buffer);
  size_t buffer_length = buffer.size();

  uint32_t crc_buffer_length =
      crc32c::Crc32c((uint8_t*)&buffer_length, sizeof(size_t));
  crc_buffer_length = TBoardFile::Mask(crc_buffer_length);

  uint32_t crc_buffer = crc32c::Crc32c((uint8_t*)buffer.data(), buffer_length);
  crc_buffer = TBoardFile::Mask(crc_buffer);

  file_.write((char*)&buffer_length, sizeof(size_t));
  file_.write((char*)&crc_buffer_length, sizeof(uint32_t));
  file_.write(buffer.data(), buffer_length);
  file_.write((char*)&crc_buffer, sizeof(uint32_t));
}

absl::StatusOr<std::string> TBoardFile::EncodeImage(std::string_view image) {
  std::ostringstream string_stream;
  std::filesystem::path image_path(image);
  if (!std::filesystem::exists(image_path)) {
    return absl::InvalidArgumentError("Image file does not exist");
  }

  std::ifstream file(image_path, std::ios::binary);
  if (!file) {
    return absl::InternalError("Failed to open image file");
  }

  string_stream << file.rdbuf();
  file.close();

  return string_stream.str();
}

// Return a masked representation of crc.
//
// Motivation: it is problematic to compute the CRC of a string that
// contains embedded CRCs.  Therefore we recommend that CRCs stored
// somewhere (e.g., in files) should be masked before being stored.
uint32_t TBoardFile::Mask(uint32_t crc) {
  // Rotate right by 15 bits and add a constant.
  return ((crc >> 15) | (crc << 17)) + kMaskDelta;
}
}  // namespace uchen::tboard