#include "uchen/tboard/tboard_file.h"

#include <filesystem>

#include <gtest/gtest.h>

TEST(FileTest, WritesLoss) {
  auto tboard_file = uchen::tboard::TBoardFile::Open(
      "events.out.tfevents");
  if (!tboard_file.ok()) {
    FAIL() << "Failed to open TBoard file: " << tboard_file.status();
  }

  for (int i = 0; i < 10; ++i) {
    tboard_file->RecordLoss(0.5 * i, i);
  }
  SUCCEED() << "Loss recorded";
}