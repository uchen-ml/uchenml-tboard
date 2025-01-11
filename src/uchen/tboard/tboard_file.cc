#include "uchen/tboard/tboard_file.h"

#include <fstream>
#include <ios>

#include "absl/log/log.h"

#include "src/uchen/tboard/tboard_file.h"

namespace uchen::tboard {

absl::StatusOr<TBoardFile> TBoardFile::Open(std::string_view path) {
  std::ofstream file(std::string(path), std::ios::binary);
  if (!file.is_open()) {
    return absl::InternalError("Failed to open file");
  }
  return TBoardFile(std::move(file));
}

void TBoardFile::RecordLoss(float loss) {
  LOG(INFO) << "Recording loss: " << loss;
  file_ << "Loss: " << loss << std::endl;
}

}  // namespace uchen::tboard