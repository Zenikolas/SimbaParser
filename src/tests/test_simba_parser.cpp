#include <gtest/gtest.h>
#include <simba_parser.h>

TEST(ParserTest, DummyTest) {
  std::vector<uint8_t> rawData;
  auto messages = simba::parse(rawData);
  EXPECT_TRUE(messages.empty());
}