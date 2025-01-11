#include <string_view>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"

#include "uchen/tboard/tboard_file.h"

ABSL_FLAG(std::string, name, "World", "The name to greet");

namespace uchen::tboard {
namespace {

constexpr std::string_view kStringConstant = "Hello";

}
}  // namespace uchen::tboard

int main(int argc, char* argv[]) {
  std::string executable(argv[0]);
  absl::SetProgramUsageMessage(absl::StrFormat(
      "Example executable that shows building and setting up ABSL\n\n"
      "  Usage: %s <output_file>",
      executable.substr(executable.find_last_of("/\\") + 1)));
  auto args = absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();
  if (args.size() != 2) {
    std::cerr << "Output file name is required\n";
    return 1;
  }
  std::string name = absl::GetFlag(FLAGS_name);
  LOG(INFO) << uchen::tboard::kStringConstant << ", " << name << "!";
  auto tboard_file = uchen::tboard::TBoardFile::Open(args[1]);
  if (!tboard_file.ok()) {
    LOG(ERROR) << "Failed to open TBoard file: " << tboard_file.status();
    return 1;
  }
  tboard_file->RecordLoss(0.5);
  return 0;
}