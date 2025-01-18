#include "uchen/tboard/tboard_file.h"

#include <fstream>
#include <ios>
#include <sstream>

#include "absl/log/log.h"

#include "crc32c/crc32c.h"
#include "proto/event.pb.h"
#include "proto/summary.pb.h"
#include "src/uchen/tboard/tboard_file.h"

namespace uchen::tboard {

TBoardFile::~TBoardFile() {
  if (file_.is_open()) {
    LOG(INFO) << "Closing log file";
    file_.close();
  }
}

absl::StatusOr<TBoardFile> TBoardFile::Open(std::string_view path) {
  if (path.empty()) {
    return absl::InvalidArgumentError("Log file name must be provided");
  }

  if (path.find(uchen::tboard::kLogKey) == std::string::npos) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "Log file name must contain \"%s\" to be processed by TensorBoard.",
        uchen::tboard::kLogKey));
  }

  std::ofstream file(std::string(path), std::ios::binary);
  if (!file.is_open()) {
    return absl::InternalError("Failed to open file");
  }
  return TBoardFile(std::move(file));
}

void TBoardFile::RecordLoss(double loss, uint32_t step) {
  LOG(INFO) << "Recording loss: " << loss;
  TBoardFile::AddScalar("loss", loss, step);
}

void TBoardFile::RecordScalar(const std::string& tag, double value,
                              uint32_t step) {
  TBoardFile::AddScalar(tag, value, step);
}

void TBoardFile::RecordImage(const std::string& tag, const std::string& image,
                             uint32_t step, uint32_t width, uint32_t height,
                             uint32_t channel, const std::string& image_name,
                             const std::string& image_description) {
  std::string encoded_image = EncodeImage(image);

  LOG(INFO) << "Recording image: " << tag;

  tensorflow::SummaryMetadata* metadata = new tensorflow::SummaryMetadata();
  if (!image_name.empty()) {
    metadata->set_display_name(image_name);
  } else {
    metadata->set_display_name(tag);
  }
  metadata->set_summary_description(image_description);

  tensorflow::Summary::Image* image_summary = new tensorflow::Summary::Image();
  image_summary->set_width(width);
  image_summary->set_height(height);
  image_summary->set_colorspace(channel);
  image_summary->set_encoded_image_string(encoded_image);

  tensorflow::Summary* summary = new tensorflow::Summary();
  tensorflow::Summary::Value* summary_value = summary->add_value();
  if (!summary_value) {
    LOG(ERROR) << "[" << tag << "] Failed to add value to summary";
    return;
  }

  summary_value->set_tag(tag);
  summary_value->set_allocated_image(image_summary);
  summary_value->set_allocated_metadata(metadata);

  AddEvent(summary, step);
}

void TBoardFile::AddScalar(const std::string& tag, double value,
                           uint32_t step) {
  LOG(INFO) << "Adding scalar: " << tag << ": " << value << ", Step: " << value;

  tensorflow::Summary* summary = new tensorflow::Summary();

  tensorflow::Summary::Value* summary_value = summary->add_value();
  if (!summary_value) {
    LOG(ERROR) << "[" << tag << "] Failed to add value to summary";
    return;
  }
  summary_value->set_tag(tag);
  summary_value->set_simple_value(value);

  AddEvent(summary, step);
}

void TBoardFile::AddEvent(tensorflow::Summary* summary, uint32_t step) {
  if (!summary) {
    LOG(ERROR) << "Failed to add event to summary";
    return;
  }

  tensorflow::Event event;
  event.set_wall_time(static_cast<double>(time(nullptr)));
  event.set_step(step);
  event.mutable_summary()->Swap(summary);

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

std::string TBoardFile::EncodeImage(const std::string& image) {
  std::ostringstream string_stream;
  std::ifstream file(image, std::ios::binary);
  if (!file) {
    LOG(ERROR) << "Failed to open image file: " << image;
    return "";
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
  return ((crc >> 15) | (crc << 17)) + uchen::tboard::kMaskDelta;
}
}  // namespace uchen::tboard