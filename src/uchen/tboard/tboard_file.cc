#include "uchen/tboard/tboard_file.h"

#include <fstream>
#include <ios>

#include "absl/log/log.h"

#include "crc32c/crc32c.h"
#include "proto/event.pb.h"
#include "proto/summary.pb.h"
#include "src/uchen/tboard/tboard_file.h"

namespace uchen::tboard {

TBoardFile::~TBoardFile() {
  if (file_.is_open()) {
    LOG(INFO) << "Closing log file: ";
    file_.close();
  }
}

absl::StatusOr<TBoardFile> TBoardFile::Open(std::string_view path) {
  std::ofstream file(std::string(path), std::ios::binary);
  if (!file.is_open()) {
    return absl::InternalError("Failed to open file");
  }
  return TBoardFile(std::move(file));
}

void TBoardFile::RecordLoss(double loss, uint32_t step) {
  LOG(INFO) << "Recording loss: " << loss;
  TBoardFile::AddScalar("loss", step, loss);
}

void TBoardFile::RecordScalar(const std::string& tag, double value,
                              uint32_t step) {
  TBoardFile::AddScalar(tag, value, step);
}

void TBoardFile::AddScalar(const std::string& tag, double value,
                           uint32_t step) {
  LOG(INFO) << "Adding scalar: " << tag << ": " << value << ", Step: " << value;

  tensorflow::Summary summary;

  tensorflow::Summary::Value* summary_value = summary.add_value();
  if (!summary_value) {
    LOG(ERROR) << "[" << tag << "] Failed to add value to summary";
    return;
  }
  summary_value->set_tag(tag);
  summary_value->set_simple_value(value);

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

// Return a masked representation of crc.
//
// Motivation: it is problematic to compute the CRC of a string that
// contains embedded CRCs.  Therefore we recommend that CRCs stored
// somewhere (e.g., in files) should be masked before being stored.
uint32_t TBoardFile::Mask(uint32_t crc) {
  // Rotate right by 15 bits and add a constant.
  return ((crc >> 15) | (crc << 17)) + TBoardFile::kMaskDelta;
}
}  // namespace uchen::tboard