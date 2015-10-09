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
GOAL: Support mm::MatchMaker.
An advanced algorithm which checks if certain groups are
overrepresented and gives advice on "Removing" certain people in groups
from the simulation (to ensure long term stability).

Generally in a good simulation the number of removals/total relation
request does not need to exceed 0.1%.

INPUT: The number of people in the normal groups and priority groups for the
next round of matching.

IMPLEMENTATION: For every group, it calculates a weighted average of the number
of people in excess of a pre-specified ratio. This average excess will then be
removed at every timestep.

OUTPUT: An advice on how many people to remove from every group. This algorithm
automatically checks if the required number of people can be removed from the 
priority group.

NOTE: This class seems imptovable
*/
#ifndef SOA1_RG_MM_ROBUSTNESS_CHECK_H
#define SOA1_RG_MM_ROBUSTNESS_CHECK_H
#include <vector>
#include "alje_historic_exponential_weighting.h"

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

class RobustnessCheck {
public:
  void Update(std::vector<int> normal, std::vector<int> priority) {
    for (int i = 0; i < n_groups_; ++i) {
      double exceeds = priority[i] - normal[i] * max_percent_in_priority_goal_;
      // We do not allow for negative exceeds
      if (exceeds < 0) exceeds = 0;
      avg_[i] = weight_new_.GetWeight(n_times_called_) * exceeds +
        (1 - weight_new_.GetWeight(n_times_called_)) * avg_[i];
    }
  }
  
  std::vector<int> Advice(std::vector<int> normal, std::vector<int> priority) {
    // We advice to remove an amount equal to the average excess. But we never
    // advice to remove below 20 (small numbers).

    std::vector<int> return_vec(n_groups_,0);
    for (int i = 0; i < n_groups_; ++i) {
      cum_remove_[i] = cum_remove_[i] + avg_[i]/1000.0; // XXX magic constant
      return_vec[i] = static_cast<int>(cum_remove_[i]); //round down.
      // See if this amount is possible to remove (and assure we always leave
      // at least 10 in the priority list).
      if (return_vec[i] > priority[i] - 10) {
        return_vec[i] = priority[i] - 10 ;
        if (return_vec[i] < 0)
          return_vec[i] = 0;
      }
      // We assume the advice is followed and the values are removed.
       cum_remove_[i] = cum_remove_[i] - return_vec[i];
    }
    return return_vec;
  }

  std::vector<int> UpdateandAdvice(std::vector<int> normal, 
                                    std::vector<int> priority) {
    ++n_times_called_;
    Update(normal, priority);
    return Advice(normal, priority);
  }

  RobustnessCheck(int n_groups, double max_percent_in_priority_goal) :
    n_groups_(n_groups),
    avg_(n_groups, 0.0), 
    cum_remove_(n_groups, 0.0),
    max_percent_in_priority_goal_(max_percent_in_priority_goal),
    weight_new_(w_){
  }

private:
  const int n_groups_;
  // Average amount it exceeds the goal.
  std::vector<double> avg_; 
  // How many we ought to remove. But since we can only remove integer
  // values we store the remainder here (in order to take the remainders 
  // together)
  std::vector<double> cum_remove_; 
  double max_percent_in_priority_goal_;
  // Which weight should place on new arrivals.
  int n_times_called_ = 0;
  double w_ = 0.01; //xxx fix this magic constant.
  alje::HistoricExponentialWeighting weight_new_;

};//!class RelationRequest
}// !namespace mm
}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_MM_ROBUSTNESS_CHECK_H