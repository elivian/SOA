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
GOAL: Provide an abstraction layer which turns relation_requests into
group numbers. And also specifies preferences between the groups.

INPUT:Relation_Requests
INPUT on construction: time&, person list& (for getting the age of people)
  and the groups as specified in the parameters.

IMPLEMENTATION: The most complex part is getting the preferences between
groups. For this (at the moment) it assumes 2 dimensions: age and duration. 
These are assumed to be independent. This allows much work to be done by the
mm::group_dimension_... classes and we only need to combine them.

OUTPUT: depends on the function
* The total number of groups
* A groupnumber
* Preferencematrix[i,j]. If someone from group i could choose any partner what
  is the probability that he chooses someone from group j?
*/

#ifndef SOA1_RG_MM_GROUP_HANDLER_H
#define SOA1_RG_MM_GROUP_HANDLER_H
#include <vector>
#include <string> // For returning the name of the group (for debugging)
#include <cassert>
#include <algorithm> // For lower_bound

#include "soa1_rg_mm_relation_request.h"
#include "soa1_rg_mm_group_dimension_normal.h"
#include "soa1_rg_mm_group_dimension_exact.h"

#include "soa1_parameters_pack.h"
#include "soa1_sv_person_list.h"

namespace soa1 { // soa is the dutch equivalent of sti.
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

class GroupHandler {
public:
   int GroupNumber(RelationRequest request) {
    int person_id = request.person_id;
    int duration = request.duration_in_days;
    double person_age_years =
      (time_ - person_list_[person_id].day_of_birth()) / 365.0;

    int age_group = age_dimension_.GroupNumber(person_age_years);
    int duration_group = duration_dimension_.GroupNumber(duration);
    return age_group * duration_dimension_.NumberOfGroups() + duration_group;
  }// !GroupNumber(...)
  
  int NumberOfGroups() {
    return age_dimension_.NumberOfGroups() * duration_dimension_.NumberOfGroups();
  }

  std::vector<std::vector<double>> PreferenceMatrix() {
    int n = this->NumberOfGroups();
    // Initialize empty matrix
    std::vector<std::vector<double>> return_matrix(n, std::vector<double>(n));
    
    // Declare some variables. dim1 (dimension 1, in this case age)
    // dim 2 (dimension 2, in this case duration)
    // o (overall, the combination of the two)
    int dim1_n_groups = age_dimension_.NumberOfGroups();
    int dim2_n_groups = duration_dimension_.NumberOfGroups();
    int o_n_groups = this->NumberOfGroups();

    // What are the preferences of both dimensions?
    auto dim1_pref = age_dimension_.PreferenceMatrix();
    auto dim2_pref = duration_dimension_.PreferenceMatrix();

    // We need to know PreferenceMatrix[o_from, o_to] so 
    // loop over all these.
    for (int o_from = 0; o_from < o_n_groups; ++o_from) {
      for (int o_to = 0; o_to < o_n_groups; ++o_to) {
        // Combine the 2. We put dim2 second and count as follows
        // so {0,0}, {0,1}, {0,2}, {1,0)... The next 4 lines do the counting
        // this way.
        int dim1_from = o_from / dim2_n_groups;
        int dim2_from = o_from % dim2_n_groups;
        int dim1_to = o_to / dim2_n_groups;
        int dim2_to = o_to % dim2_n_groups;

        // No we properly looped over everything and we can use our assumption
        // of independence to get the overall preference matrix from the
        // preferences on the seperate dimensions.
        return_matrix[o_from][o_to] = 
            dim1_pref[dim1_from][dim1_to] * dim2_pref[dim2_from][dim2_to];
      }
    }
    // And we are done :).
    return return_matrix;
   }// !PreferenceMatrix

    std::string GroupName(int group_nr) {
    int age_group = group_nr / duration_dimension_.NumberOfGroups();
    int duration_group = group_nr % duration_dimension_.NumberOfGroups();

    return "Age group: " + age_dimension_.Name(age_group) = " Duration group: "
      + duration_dimension_.Name(duration_group);
  }

  GroupHandler(int& time, soa1::sv::PersonList& person_list, 
    soa1::parameters::MatchMaking mm_parameters) : 
      time_(time), 
      person_list_(person_list),
      age_dimension_(mm_parameters.age_groups,
          mm_parameters.age_group_preference_sd),
      duration_dimension_(mm_parameters.duration_groups)
  {
    assert(mm_parameters.age_group_preference_distribution == "normal"
      && "Error in soa1_rg_mm_group_handler->Constructor in the parameter "
      "pack someone specified a non-normal distribution for age group "
      "distribution. Unfortunately this is not supported. I suggest you "
      "either change it into a normal distribution or change this code.");
    assert(mm_parameters.duration_group_preference_distribution == "exact"
      && "Error in soa1_rg_mm_group_handler->Constructor in the parameter "
      "pack someone specified a non-exact distribution for duration groups "
      "distribution. Unfortunately this is not supported. I suggest you "
      "either change it into an exact fit (only in-duration group matches)"
      "or change this code.");
    // Other asserts are done in the GroupDimension classes
  }

  GroupHandler() = delete; // we need MatchMaking Parameters!
private:
  int& time_; // Needed to calculate someone's age
  // Needed because we get personID but want age.
  soa1::sv::PersonList& person_list_; 
  mm::GroupDimensionNormal age_dimension_;
  mm::GroupDimensionExact  duration_dimension_;
};//!class GroupHandler
}// !namespace mm
}// !namespace rg
}// !namespace soa1
#endif // !SOA1_RG_MM_GROUP_HANDLER_H