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
GOAL: We want to divide relation_requests into groups (see group_handler for
more details on this). 

But often the division into groups will be independent of the other groups (so
the groups depending on age might not affect the division into groups depending
on ethnicity)

So P(from age group 15-20 && wants someone from age 20-25 && from white 
  ethnicity && wants hispanic ethnicity) 
  = P(from age group 15-20 && wants someone from age 25) * P(from white 
  ethnicity && wants hispanic ethnicity)

group_dimension_normal takes into account 1 division of groups (for example age, 
duration or sexual activity) and will assume the preferences of people are
normally distributed (so the preferred age for someone of age 25 is normally
distributed with mean 25 and a fixed standard deviation).

INPUT: depends on function.

INPUT ON CONSTRUCTION: The groups, the sd.
std::vector<std::vector<double>> age_groups{
{15,20},
{20,25},
...
{75,80}
};
It is assumed to be upper_including.

IMPLEMENTATION: So from the construction (assume sd = 6) we know that the age
preference of someone of 17 years old is normally distributed with mean 17 and
sd of 6. But we need to compute the % of people BETWEEN 15-20 wanting someone
from 20-25. We solve this problem by sampling a large number of people between
15 and 20 and for each computing the probability (= area of normal 
distribution) is between 20 and 25.

OUTPUT: depends on the funtion
* preference matrix (see grouphandler for more information)
* The number of groups in this dimension
* The group someone belongs to.
* The name of the group

*/

#ifndef SOA1_RG_MM_GROUP_DIMENSION_NORMAL_H
#define SOA1_RG_MM_GROUP_DIMENSION_NORMAL_H
#include <vector>
#include <string>     // For returning the name of the group 
#include <cassert>
#include <algorithm>  // For lower_bound & is_sorted
#include <numeric>    // For std::accumulate

#include <boost/math/distributions/normal.hpp>

namespace soa1 { // soa is the dutch equivalent of sti.
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

class GroupDimensionNormal {
public:
  int GroupNumber(double value) const {
    assert(value >= g_lower_.front() && value <= g_upper_.back() && "Error in "
      "soa1::rg::mm::GroupDimensionNormal->GroupNumber a value has been passed"
      " which isn't in any group." );

    // Subtracting random iterators gives an integer which is the distance.
    return std::lower_bound(g_upper_.begin(), g_upper_.end(), value)
      - g_upper_.begin();
  }// !Groupnumber(...)

  std::vector<std::vector<double>> PreferenceMatrix() const {
    // See grouphandler for details on what a preferencematrix is.    
    /*
    In order to get the values in the preference matrix (P(someone from 
    group i would prefer someone from group j)). We sample a lot of people from
    different ages and construct the preference accordingly. This to me seems
    to be more accurate than taking either a single normal distribution with
    the mean equal to the mean of the group or any other option.    
    */

    // Empty initialize
    std::vector<std::vector<double>> return_matrix;
    return_matrix.resize(this->NumberOfGroups()); 
    for (auto& group : return_matrix)
      group.resize(this->NumberOfGroups());

    // Initialize lower/upper and such
    double lower = g_lower_[0];
    double upper = g_upper_[this->NumberOfGroups() - 1];
    // The number of samples has been determined in by making it as large as 
    // possible without having an impact on performance. Testing in debug mode
    // shows that a single sample takes +- 0.15ms, therefore 100 seems to have
    // a negligible impact.
    double total_n_samples = 100 * NumberOfGroups();
    double stepsize = (upper - lower) / total_n_samples; 

    // Yes, the next step does not equal the actual number of samples. 
    // But it is close enough. For sake of simplicity you can think of the
    // "val" variable as an age (but could really be anything).
    for (double val = lower + stepsize ; val < upper; val += stepsize) {
      for (int group_to = 0; group_to < this->NumberOfGroups(); ++group_to) {
        return_matrix[this->GroupNumber(val)][group_to] 
            += this->UseNormal(val, g_lower_[group_to], g_upper_[group_to]);
      }
    }
    
    // Everything would be allright, but the rows in the above matrix might not
    // sum to 1. They should because everyone needs to have a preference. They
    // might not sum to 1 because some people might prefer people from groups
    // which do not exist (for example men under 15 might not be included in
    // the model).
    for (auto& row : return_matrix) {
      double row_sum = std::accumulate(row.begin(), row.end(), 0.0);
      for (auto& cell : row) {
        cell = cell / row_sum;
      }
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

  GroupDimensionNormal(std::vector<std::vector<double>> groups, double sd)
      : normal_dist_(0, sd) {
    
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
      assert(groups[i][0] < groups[i][1] && "Error in soa1::rg::mm::Group"
        "DimensionNormal->Constructor has been called with groups which have "
        "a lower upper bound than lower bound.");
      // Make sure that the lower bound of a group equals the upper bound
      // of the group below.    
      if (i != 0) {
        assert(groups[i][0] == groups[i - 1][1] && "Error in soa1::rg::mm::Group"
          "DimensionNormal->Constructor has been called with improperly "
          "groups. This might be overlapping groups, non-fitting groups.");
      }
    }

    for (std::vector<double> group_range : groups) {
      g_lower_.push_back(group_range[0]);
      g_upper_.push_back(group_range[1]);
    }

    assert(std::is_sorted(g_lower_.begin(), g_lower_.end())
        && std::is_sorted(g_upper_.begin(), g_upper_.end()) && "Error in soa1"
        "::rg::mm::GroupDimensionNormal->Constructor has been called with non-"
        "ascending groups.");
  }// !constructor

private:
  std::vector<double> g_lower_; // The lower limits of the groups (n.including)
  std::vector<double> g_upper_; // The upper limits of the groups (including).
  boost::math::normal normal_dist_;

  double UseNormal(double mean, double lower, double upper) const {
    // Calculates the probability that a normal distribution with the at
    // construction specified standard deviation returns a value between
    // lower and upper. 
    return boost::math::cdf(normal_dist_, upper - mean) - 
           boost::math::cdf(normal_dist_, lower - mean);
  }// !UseNormal(...)
};//!class GroupDimensionNormal
}// !namespace mm
}// !namespace rg
}// !namespace soa1

#endif// !SOA1_RG_MM_GROUP_DIMENSION_NORMAL_H
