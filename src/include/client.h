#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <queue>
#include <set>
#include <random>
#include <algorithm>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// AI Client global variables for game state tracking
std::vector<std::vector<int>> visible_map;      // -1: unknown, 0-8: mine count, 9: mine (when visited)
std::vector<std::vector<bool>> is_marked;       // True if grid is marked as mine
std::vector<std::vector<double>> mine_probability; // Probability of each grid being a mine
int total_cells_visited = 0;
int total_mines_marked = 0;
bool first_move = true;

// Helper function declarations
void updateMineProbabilities();
void performAdvancedPatternAnalysis();
bool isSubset(const std::vector<std::pair<int, int>>& a, const std::vector<std::pair<int, int>>& b);
std::vector<std::pair<int, int>> getDifference(const std::vector<std::pair<int, int>>& b,
                                               const std::vector<std::pair<int, int>>& a);
std::pair<int, int> findObviousSafe();
std::pair<int, int> findObviousMine();
std::pair<int, int> findBestAutoExplore();
std::pair<int, int> findSafestCell();

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  visible_map.assign(rows, std::vector<int>(columns, -1));
  is_marked.assign(rows, std::vector<bool>(columns, false));
  mine_probability.assign(rows, std::vector<double>(columns, 0.5));
  total_cells_visited = 0;
  total_mines_marked = 0;
  first_move = true;

  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
  first_move = false;
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  // Read the current map state from stdin
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      char c = line[j];
      if (c == '?') {
        if (visible_map[i][j] != -1) {
          // This cell was previously visible but is now unknown
          // This shouldn't happen in normal gameplay
        }
        visible_map[i][j] = -1;
      } else if (c == '@') {
        // Marked mine
        is_marked[i][j] = true;
        visible_map[i][j] = -1;
      } else if (c == 'X') {
        // Visited mine (game over)
        visible_map[i][j] = 9;
      } else if (c >= '0' && c <= '8') {
        // Visited non-mine with mine count
        visible_map[i][j] = c - '0';
        is_marked[i][j] = false;
        total_cells_visited++;
      }
    }
  }

  // Update mine probabilities based on new information
  updateMineProbabilities();
}

// Helper function to update mine probabilities with advanced pattern recognition
void updateMineProbabilities() {
  // Reset probabilities
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] == -1 && !is_marked[i][j]) {
        mine_probability[i][j] = 0.5; // Default probability
      }
    }
  }

  // Apply constraints based on numbered cells
  bool changed = true;
  while (changed) {
    changed = false;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (visible_map[i][j] >= 0 && visible_map[i][j] <= 8) {
          int count = visible_map[i][j];
          std::vector<std::pair<int, int>> neighbors;
          int marked_mines = 0;
          int unknown_cells = 0;

          // Check all 8 neighbors
          int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
          int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

          for (int k = 0; k < 8; k++) {
            int ni = i + dr[k];
            int nj = j + dc[k];
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (is_marked[ni][nj]) {
                marked_mines++;
              } else if (visible_map[ni][nj] == -1) {
                neighbors.push_back({ni, nj});
                unknown_cells++;
              }
            }
          }

          int remaining_mines = count - marked_mines;
          if (remaining_mines < 0) remaining_mines = 0;

          // If remaining_mines == 0, all unknown neighbors are safe
          if (remaining_mines == 0 && unknown_cells > 0) {
            for (auto& [ni, nj] : neighbors) {
              if (mine_probability[ni][nj] != 0.0) {
                mine_probability[ni][nj] = 0.0;
                changed = true;
              }
            }
          }
          // If remaining_mines == unknown_cells, all unknown neighbors are mines
          else if (remaining_mines == unknown_cells && unknown_cells > 0) {
            for (auto& [ni, nj] : neighbors) {
              if (mine_probability[ni][nj] != 1.0) {
                mine_probability[ni][nj] = 1.0;
                changed = true;
              }
            }
          }
        }
      }
    }
  }

  // Advanced pattern analysis: check for logical deductions between adjacent numbered cells
  performAdvancedPatternAnalysis();
}

// Advanced pattern analysis for complex logical deductions
void performAdvancedPatternAnalysis() {
  // Pattern 1: Check for 1-2 patterns and other common Minesweeper patterns
  int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

  // Find all numbered cells
  std::vector<std::tuple<int, int, int, std::vector<std::pair<int, int>>>> numbered_cells;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] >= 1 && visible_map[i][j] <= 8) {
        std::vector<std::pair<int, int>> neighbors;
        int marked_count = 0;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (is_marked[ni][nj]) {
              marked_count++;
            } else if (visible_map[ni][nj] == -1) {
              neighbors.push_back({ni, nj});
            }
          }
        }

        int remaining_mines = visible_map[i][j] - marked_count;
        if (remaining_mines > 0 && !neighbors.empty()) {
          numbered_cells.push_back({i, j, remaining_mines, neighbors});
        }
      }
    }
  }

  // Pattern analysis: look for subset relationships between cells
  for (size_t a = 0; a < numbered_cells.size(); a++) {
    for (size_t b = a + 1; b < numbered_cells.size(); b++) {
      auto [i1, j1, mines1, neighbors1] = numbered_cells[a];
      auto [i2, j2, mines2, neighbors2] = numbered_cells[b];

      // Check if neighbors1 is a subset of neighbors2
      if (isSubset(neighbors1, neighbors2)) {
        // If mines1 == mines2, then neighbors2 - neighbors1 are safe
        if (mines1 == mines2) {
          auto diff = getDifference(neighbors2, neighbors1);
          for (auto& [ni, nj] : diff) {
            mine_probability[ni][nj] = 0.0;
          }
        }
        // If mines2 == mines1 + |neighbors2| - |neighbors1|, then neighbors2 - neighbors1 are mines
        else if (mines2 == mines1 + (int)neighbors2.size() - (int)neighbors1.size()) {
          auto diff = getDifference(neighbors2, neighbors1);
          for (auto& [ni, nj] : diff) {
            mine_probability[ni][nj] = 1.0;
          }
        }
      }
    }
  }
}

// Helper function to check if vector A is subset of vector B
bool isSubset(const std::vector<std::pair<int, int>>& a, const std::vector<std::pair<int, int>>& b) {
  for (auto& cell : a) {
    bool found = false;
    for (auto& bcell : b) {
      if (cell.first == bcell.first && cell.second == bcell.second) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

// Helper function to get B - A (elements in B not in A)
std::vector<std::pair<int, int>> getDifference(const std::vector<std::pair<int, int>>& b,
                                               const std::vector<std::pair<int, int>>& a) {
  std::vector<std::pair<int, int>> diff;
  for (auto& bcell : b) {
    bool found = false;
    for (auto& acell : a) {
      if (bcell.first == acell.first && bcell.second == acell.second) {
        found = true;
        break;
      }
    }
    if (!found) {
      diff.push_back(bcell);
    }
  }
  return diff;
}

// Helper function to find obvious safe cells
std::pair<int, int> findObviousSafe() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] >= 0 && visible_map[i][j] <= 8) {
        int count = visible_map[i][j];
        std::vector<std::pair<int, int>> neighbors;
        int marked_mines = 0;
        int unknown_cells = 0;

        // Check all 8 neighbors
        int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (is_marked[ni][nj]) {
              marked_mines++;
            } else if (visible_map[ni][nj] == -1) {
              neighbors.push_back({ni, nj});
              unknown_cells++;
            }
          }
        }

        int remaining_mines = count - marked_mines;
        if (remaining_mines == 0 && unknown_cells > 0) {
          return neighbors[0]; // Return first safe cell
        }
      }
    }
  }
  return {-1, -1}; // No obvious safe cell found
}

// Helper function to find obvious mines
std::pair<int, int> findObviousMine() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] >= 0 && visible_map[i][j] <= 8) {
        int count = visible_map[i][j];
        std::vector<std::pair<int, int>> neighbors;
        int marked_mines = 0;
        int unknown_cells = 0;

        // Check all 8 neighbors
        int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (is_marked[ni][nj]) {
              marked_mines++;
            } else if (visible_map[ni][nj] == -1) {
              neighbors.push_back({ni, nj});
              unknown_cells++;
            }
          }
        }

        int remaining_mines = count - marked_mines;
        if (remaining_mines == unknown_cells && unknown_cells > 0) {
          return neighbors[0]; // Return first obvious mine
        }
      }
    }
  }
  return {-1, -1}; // No obvious mine found
}

// Helper function to find best auto-explore opportunity
std::pair<int, int> findBestAutoExplore() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] >= 1 && visible_map[i][j] <= 8) {
        int count = visible_map[i][j];
        int marked_mines = 0;
        int unknown_cells = 0;

        // Check all 8 neighbors
        int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (is_marked[ni][nj]) {
              marked_mines++;
            } else if (visible_map[ni][nj] == -1) {
              unknown_cells++;
            }
          }
        }

        // If all mines are marked, we can auto-explore
        if (marked_mines == count && unknown_cells > 0) {
          return {i, j};
        }
      }
    }
  }
  return {-1, -1}; // No auto-explore opportunity found
}

// Helper function to find safest unknown cell
std::pair<int, int> findSafestCell() {
  double min_prob = 1.0;
  std::pair<int, int> safest = {-1, -1};

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] == -1 && !is_marked[i][j]) {
        if (mine_probability[i][j] < min_prob) {
          min_prob = mine_probability[i][j];
          safest = {i, j};
        }
      }
    }
  }

  return safest;
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // Priority 1: Auto-explore if possible (most efficient)
  auto auto_explore = findBestAutoExplore();
  if (auto_explore.first != -1) {
    Execute(auto_explore.first, auto_explore.second, 2);
    return;
  }

  // Priority 2: Mark obvious mines
  auto obvious_mine = findObviousMine();
  if (obvious_mine.first != -1) {
    Execute(obvious_mine.first, obvious_mine.second, 1);
    total_mines_marked++;
    return;
  }

  // Priority 3: Visit obvious safe cells
  auto obvious_safe = findObviousSafe();
  if (obvious_safe.first != -1) {
    Execute(obvious_safe.first, obvious_safe.second, 0);
    return;
  }

  // Priority 4: Choose the safest cell based on probability
  auto safest = findSafestCell();
  if (safest.first != -1) {
    Execute(safest.first, safest.second, 0);
    return;
  }

  // Fallback: choose a random unknown cell (should not happen)
  std::vector<std::pair<int, int>> unknown_cells;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (visible_map[i][j] == -1 && !is_marked[i][j]) {
        unknown_cells.push_back({i, j});
      }
    }
  }

  if (!unknown_cells.empty()) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, unknown_cells.size() - 1);
    auto chosen = unknown_cells[dis(gen)];
    Execute(chosen.first, chosen.second, 0);
  }
}

#endif