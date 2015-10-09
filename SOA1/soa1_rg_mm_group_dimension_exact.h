/* SOA1, individual based STI simulation
Copyright (C) 2015, RIVM

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

/*
GOAL: Division in groups and allow for only within-group preferences (hence exact)

NOTE: See group_dimension_normal for more details. (also on functions)
*/
#ifndef SOA1_RG_MM_GROUP_DIMENSION_EXACT_H
#define SOA1_RG_MM_GROUP_DIMENSION_EXACT_H
#include <vector>
#include <string> // For returning the name of the group (for debugging)
#include <cassert>
#include <algorithm> // For lower_bound

namespace soa1 { // soa is the dutch equivalent of sti.
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

class GroupDimensionExact{
public:
   int GroupNumber(int value) {
    assert(value >= lowest_ && value <= highest_ && "Error in "
      "soa1::rg::mm::GroupDimensionExact->GroupNumber a value has been passed "
      "which isn't in any group.");

    // Subtracting random iterators gives an integer which is the distance.
    return std::lower_bound(g_upper_.begin(), g_upper_.end(), value) 
        - g_upper_.begin();
  }

  std::vector<std::vector<double>> PreferenceMatrix() const {
    std::vector<std::vector<double>> return_matrix;
    for (int row = 0; row < this->NumberOfGroups(); ++row) {
      std::vector<double> row_vec;
      for (int column = 0; column < this->NumberOfGroups(); ++column) {
        if (row == column) {
          row_vec.push_back(1); // Yes we want someone from our own group
        } else {
          row_vec.push_back(0); // No we don't want someone from another group.
        }
      }
      return_matrix.push_back(row_vec);
    }
    return return_matrix;
  }// !PreferenceMatrix()

  std::string Name(int group) {
    return std::to_string(g_lower_[group])
      + "-"
      + std::to_string(g_upper_[group]);
  }// !Name(...)

  int NumberOfGroups() const {
    return static_cast<int>(g_upper_.size());
  }// !NumberOfGroups()

  GroupDimensionExact(std::vector<std::vector<int>> groups) {
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
      assert(groups[i][0] <= groups[i][1] && "Error in soa1::rg::mm::Group"
        "DimensionExact->Constructor has been called with groups which have "
        "a lower upper bound than lower bound.");
      // Make sure that the lower bound of a group equals the upper bound + 1
      // of the group below.    
      if (i != 0) {
        assert(groups[i][0] == groups[i - 1][1]+1 && "Error in soa1::rg::mm::Group"
          "DimensionExact->Constructor has been called with improperly "
          "groups. This might be overlapping groups, non-fitting groups.");
      }
    }
    // The lowest value of the lowest group is the lowest value we can
    // receive.
    lowest_ = groups.front().front(); // Front is the first so lowest. 
    highest_ = groups.back().back();
    for (std::vector<int> group_range : groups) {
      g_lower_.push_back(group_range.front());
      g_upper_.push_back(group_range.back()); 
    }
    assert(std::is_sorted(g_upper_.begin(), g_upper_.end()) && "Error in soa1"
      "::rg::mm::GroupDimensionExact>Constructor has been called with non-"
      "ascending groups.");
  }// !Constructor

private:
  std::vector<int> g_lower_;
  std::vector<int> g_upper_; // The upper limits of the groups (including).
  int lowest_; // Lowest value still allowed to be in a group.
  int highest_;// Highest value still allowed to be in a group.
};//!class GroupDimensionExact
}// !namespace mm
}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_MM_GROUP_DIMENSION_EXACT_H
