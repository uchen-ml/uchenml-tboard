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

TEST(FileTest, WritesScalar) {
  auto tboard_file = uchen::tboard::TBoardFile::Open(
      "events.out.tfevents");
  if (!tboard_file.ok()) {
    FAIL() << "Failed to open TBoard file: " << tboard_file.status();
  }

  for (int i = 0; i < 10; ++i) {
    tboard_file->RecordScalar("accuracy", 0.5 * i, i);
  }
  SUCCEED() << "Scalar recorded";
}

TEST(FileTest, WritesImage) {
  auto tboard_file = uchen::tboard::TBoardFile::Open(
      "events.out.tfevents");
  if (!tboard_file.ok()) {
    FAIL() << "Failed to open TBoard file: " << tboard_file.status();
  }

  // TODO: Handle relative paths
  std::string image_path = "014_1_0012.png";
  tboard_file->RecordImage("traffic_sign", image_path, 1, 148, 135, 3,
                           "Traffic Sign", "Traffic Sign Image");
  SUCCEED() << "Image recorded";
}