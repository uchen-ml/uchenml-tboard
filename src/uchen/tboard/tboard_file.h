#ifndef UCHEN_TBOARD_TBOARD_FILE_H_
#define UCHEN_TBOARD_TBOARD_FILE_H_

#include <fstream>
#include <string>
#include <string_view>

#include "absl/status/statusor.h"

#include "proto/summary.pb.h"

namespace uchen::tboard {

// MaskDelta is a fixed arbitrary number that is used to mask the CRC32
// checksum. expected from TensorBoard.
constexpr uint32_t kMaskDelta = 0xa282ead8ul;

constexpr std::string_view kLogKey = "tfevents";

class TBoardFile {
 public:
  static absl::StatusOr<TBoardFile> Open(std::string_view path);

  TBoardFile(TBoardFile&&) = default;
  ~TBoardFile();

  void RecordLoss(double loss, uint32_t step);
  void RecordScalar(const std::string& tag, double value, uint32_t step);

  void RecordImage(const std::string& tag, const std::string& image,
                   uint32_t step, uint32_t width, uint32_t height,
                   uint32_t channel, const std::string& image_name,
                   const std::string& image_description);

 private:
  TBoardFile(std::ofstream file) : file_(std::move(file)) {}

  void AddScalar(const std::string& tag, double value, uint32_t step);
  void AddEvent(tensorflow::Summary* summary, uint32_t step);

  std::string EncodeImage(const std::string& image);

  uint32_t Mask(uint32_t crc);

  std::ofstream file_;
};

}  // namespace uchen::tboard

#endif  // UCHEN_TBOARD_TBOARD_FILE_H_