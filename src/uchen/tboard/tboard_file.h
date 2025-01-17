#ifndef UCHEN_TBOARD_TBOARD_FILE_H_
#define UCHEN_TBOARD_TBOARD_FILE_H_

#include <fstream>
#include <string_view>

#include "absl/status/statusor.h"

namespace uchen::tboard {

class TBoardFile {
 public:
  static absl::StatusOr<TBoardFile> Open(std::string_view path);

  TBoardFile(TBoardFile&&) = default;
  ~TBoardFile();

  void RecordLoss(double loss, uint32_t step);
  void RecordScalar(const std::string& tag, double value, uint32_t step);

 private:
  TBoardFile(std::ofstream file) : file_(std::move(file)) {}

  void AddScalar(const std::string& tag, double value, uint32_t step);

  uint32_t Mask(uint32_t crc);

  // MaskDelta is a fixed arbitrary number that is used to mask the CRC32
  // checksum. expected from TensorBoard.
  const uint32_t kMaskDelta = 0xa282ead8ul;

  std::ofstream file_;
};

}  // namespace uchen::tboard

#endif  // UCHEN_TBOARD_TBOARD_FILE_H_