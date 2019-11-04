#include <algorithm>
#include <random>
#include <memory>
#include <chrono>
#include <iostream>
#include <map>
#include <unordered_map>
#include <utility>

namespace util {
auto seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 random_engine_{seed};

template<typename Iter>
decltype(auto) SelectRandomly(Iter start, Iter end) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(random_engine_));
  return *start;
}

template<typename C>
decltype(auto) Tolower(C c) {
  std::transform(begin(c),
                 end(c),
                 begin(c),
                 [](unsigned char c) { return std::tolower(c); });
  return c;
}

using History = std::vector<int>;
}

namespace approx {

using namespace util;

struct Game {
  enum class Player {
    NONE, BLACK, WHITE, DRAW
  };

  virtual ~Game() = default;

  /**
   * Do a random play to the game
   *
   * @return The position it played
   */
  virtual int Play() = 0;

  /**
   * Check game termination.
   *
   * @return True, if the game is terminated. False, otherwise.
   */
  virtual bool CheckTermination() = 0;

  /**
   * Get the winner of the game.
   *
   * @return A string indicating who is the winner.
   */
  virtual std::string Winner() = 0;

  virtual Player GetCurrentPlayer() = 0;
};

class Tictactoe : public Game {
  const int k_board_width_ = 3;
  const int k_board_height_ = 3;
  const int k_num_connect_ = 3;

 public:
  int Play() override {
    // find empty positions
    std::vector<int> empty_pos;
    for (size_t i = 0; i < board_.size(); ++i) {
      if (board_[i] == Player::NONE) {
        empty_pos.push_back(i);
      }
    }

    // put a piece into one of the empty positions
    int result = SelectRandomly(std::begin(empty_pos),
                                std::end(empty_pos));

    board_[result] = current_player_;
    history_.push_back(result);
    current_player_ = NextPlayer();

    return result;
  }

  bool CheckTermination() override {
    if (win_player_ != Player::NONE) {
      return true;
    }

    if (ContinuousCount(ExtendingDirection::SLASH) >= k_num_connect_ ||
        ContinuousCount(ExtendingDirection::BACK_SLASH) >= k_num_connect_ ||
        ContinuousCount(ExtendingDirection::VERTICAL) >= k_num_connect_ ||
        ContinuousCount(ExtendingDirection::HORIZONTAL) >= k_num_connect_) {
      win_player_ = board_[history_.back()];
      return true;
    }

    // An full board meets the terminate condition
    if (history_.size() == board_.size()) {
      win_player_ = Player::DRAW;
      return true;
    }

    return false;
  }

  std::string Winner() override {
    if (win_player_ == Player::BLACK) {
      return "BLACK";
    } else if (win_player_ == Player::WHITE) {
      return "WHITE";
    } else if (win_player_ == Player::DRAW) {
      return "DRAW";
    } else {
      return "UNKNOWN";
    }
  }

  Player GetCurrentPlayer() override {
    return current_player_;
  }
 private:
  std::vector<Player>
      board_ = std::vector<Player>(k_board_width_ * k_board_height_);
  Player current_player_ = Player::BLACK;
  Player win_player_ = Player::NONE;
  History history_;
 private:
  enum class ExtendingDirection { SLASH, BACK_SLASH, VERTICAL, HORIZONTAL };
  int ContinuousCount(ExtendingDirection e) {
    // No history available, the board is empty, there must have exactly `0` continuous counts
    if (history_.empty())
      return 0;

    int x_step = 0, y_step = 0;
    if (e == ExtendingDirection::SLASH) {
      x_step = 1;
      y_step = -1;
    } else if (e == ExtendingDirection::BACK_SLASH) {
      x_step = 1;
      y_step = 1;
    } else if (e == ExtendingDirection::VERTICAL) {
      x_step = 0;
      y_step = 1;
    } else if (e == ExtendingDirection::HORIZONTAL) {
      x_step = 1;
      y_step = 0;
    }
    // We only do winning checks on the latest (last) applied action instead
    // of repeatedly checking for winning conditions.
    const Player kLatestPlayer = board_[history_.back()];
    const int kX = history_.back() % k_board_width_;
    const int kY = history_.back() / k_board_width_;

    int count = 1;
    for (int extended_x = kX + x_step, extended_y = kY + y_step;;) {

      if (!IsInBoundary(extended_x, extended_y)) {
        break;
      }
      if (PlayerAt(extended_x, extended_y) != kLatestPlayer) {
        break;
      }
      extended_x += x_step;
      extended_y += y_step;
      count += 1;
    }

    // Also inspect on another side
    x_step *= -1;
    y_step *= -1;

    for (int extended_x = kX + x_step, extended_y = kY + y_step;;) {
      if (!IsInBoundary(extended_x, extended_y)) {
        break;
      }
      if (PlayerAt(extended_x, extended_y) != kLatestPlayer) {
        break;
      }
      extended_x += x_step;
      extended_y += y_step;
      count += 1;
    }
    return count;
  }
  inline bool IsInBoundary(int x, int y) const {
    return 0 <= x && x < k_board_width_
        && 0 <= y && y < k_board_height_;
  }
  inline Player PlayerAt(int x, int y) const {
    return static_cast<Player>(board_[y * k_board_width_ + x]);
  }
  Player NextPlayer() const {
    if (current_player_ == Player::BLACK) {
      return Player::WHITE;
    } else if (current_player_ == Player::WHITE) {
      return Player::BLACK;
    } else {
      // Somethings bad might just happened,
      // current_player should always be switching between white or black
      return Player::NONE;
    }
  }

};

std::unique_ptr<Game> Factory(std::string game_name) {
  game_name = Tolower(game_name);

  std::unique_ptr<Game> result;
  if (game_name == "tictactoe") {
    result = std::make_unique<Tictactoe>();
  } else {
    // result = std::make_unique<Base>();
  }

  return result;
}

class GameController {
 public:
  explicit GameController(std::string str) : game_name_(std::move(str)) {};

  void PlayOneTime() {
    std::unique_ptr<Game> game = Factory(game_name_);
    while (!game->CheckTermination()) {
      int result = game->Play();
    }
    result_stats_[game->Winner()] += 1;
  }

  void PrintResult() {
    for (const auto &i : result_stats_) {
      std::cout << "Player = " << i.first << ", Win " << i.second << std::endl;
    }
  }

 private:
  std::unordered_map<std::string, int> result_stats_{};
  std::string game_name_;
};
} // namespace Approx

int main(int argc, char **argv) {
  // Initialize a game with a name
  approx::GameController game_controller{"Tictactoe"};

  // Play a game for 1e5 time
  for (int i = 0; i < 1e5; ++i) {
    game_controller.PlayOneTime();
  }

  game_controller.PrintResult();
}
