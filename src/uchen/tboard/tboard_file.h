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
  ~TBoardFile() = default;

  void RecordLoss(float loss);

 private:
  TBoardFile(std::ofstream file) : file_(std::move(file)) {}

  std::ofstream file_;
};

}  // namespace uchen::tboard

#endif  // UCHEN_TBOARD_TBOARD_FILE_H_