#ifndef UCHEN_TBOARD_TBOARD_FILE_H_
#define UCHEN_TBOARD_TBOARD_FILE_H_

#include <fstream>
#include <string>
#include <string_view>

#include "absl/status/statusor.h"

#include "proto/summary.pb.h"

namespace uchen::tboard {

class TBoardFile {
 public:
  static absl::StatusOr<TBoardFile> Open(std::string_view logs_directory,
                                         std::string_view log_name="events");

  TBoardFile(TBoardFile&&) = default;
  ~TBoardFile() = default;

  void AddScalar(const std::string& tag, double value, uint32_t step);

  void AddImage(const std::string tag, const std::string& image,
                   uint32_t step, uint32_t width, uint32_t height,
                   uint32_t channel, const std::string& image_name,
                   const std::string& image_description);

 private:
  TBoardFile(std::ofstream file) : file_(std::move(file)) {}

  void AddEvent(tensorflow::Summary& summary, uint32_t step);

  absl::StatusOr<std::string> EncodeImage(std::string_view image);

  uint32_t Mask(uint32_t crc);

  std::ofstream file_;
};

}  // namespace uchen::tboard

#endif  // UCHEN_TBOARD_TBOARD_FILE_H_