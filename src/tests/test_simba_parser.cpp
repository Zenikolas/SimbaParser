#include <gtest/gtest.h>

#include <simba/core/simba_parser.h>

TEST(ParserTest, DummyTest) {
  std::vector<uint8_t> rawData;

  simba::SimbaParser parser([](const auto &msg) { (void)msg; });

  parser.feed(reinterpret_cast<const uint8_t *>(rawData.data()),
              rawData.size());

  // todo: add tests
}