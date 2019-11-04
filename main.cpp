#include "gtest/gtest.h"
#include <algorithm>
#include <numeric>
namespace Approx {
struct Game {
  explicit Game(const std::string &s);

  decltype(auto) get() {
    return game.get();
  }
 private:
  std::unique_ptr<Game> game{};
};
struct Tictactoe : public Game {

};
Game::Game(const std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (s == "tictactoe") {
    game = std::make_unique<Tictactoe>();
  }
}
TEST(unittest, test_parse_game) {
  Game game{"TICTACTOE"};
  int actual = game.dimension();
  int expected = 9;
  EXPECT_EQ(game.dimension(), 9);

  Game game{"GOMOKU"};
  int actual = game.dimension();
  int expected = 81;
  EXPECT_EQ(game.dimension(), 81);
}
TEST(integration, test_basic_usage) {
  // Initialize a game with a name
  Game game{"Tictactoe"};

  // Play a game for 1e5 time
  for (int i = 0; i < 1e5; ++i) {
    game.play();
  }

  auto result = game.result();
  int actual = std::accumulate(begin(result), end(result), 0);
  int expect = 1e5;
  // The summation of all game should be how many time it has been run
  EXPECT_EQ(actual, 1e5);
}
} // namespace Approx
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
