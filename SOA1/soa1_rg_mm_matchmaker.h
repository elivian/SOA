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
GOAL: Matchmaker is the class which decides who gets a relation with whom.

AddRelationRequest(): is called if a there is a new person which wants to
be matched to another person.
Get(): returns a vector with the best matches.

INPUT: people to schedule for a relation (id & requested relation duration)

INPUT ON CONSTRUCTION: 
A class which handles all group information 
- unique hash Function from (person_id, duration) -> groupnr
- A PreferenceMatrix[i][j] function which returns a vector<vector<double>> 
  which holds the probability that someone from group i will get a relation
  with someone from group j
- A NumberOfGroups() function returning the number of groups
- A GetName printable name function (for debugging) 

IMPLEMENTATION:
* Grouphandler gives all the information about which groups people belong to
  and who those people prefer. 
* PartnerChoiceMatrix keeps track of the reality. How many people are in every
  group and is responsible for turning the PreferenceMatrix (what people would
  like to happen) into the PartnerChoiceMatrix (what actually can happen).
* RobustnessCheck gives advice on which people to remove from the population
  in order to keep everything stable
* Linkhandler: Stores for every link: 
  E[number of times it should have been scheduled] - #times scheduled. 
  So the ones with the highest value (at the top) are the links we want to 
  occur most badly. So at every timestep we loop through the linkhandler from
  top to bottom to see if we can schedule anything. It turns out that once we
  passed any link and could not schedule this we will not be able to schedule
  that link at this timestep so we continue. We first do this process in
  "priority-mode" accepting any link which schedules at least 1 relation
  request from the priority group. (a relation request gets the priority status
  if it couldn't be scheduled on the first day).
Get() is the most complex function. 
* Keep synchronized with the PartnerChoiceMatrix class
    - Let PartnerChoiceMatrix know how many people are added to each group
      so we can get overall %. (step 1)
    - Check if the partner choice matrix is updated (because #people in groups
      might change the partner choice matrix might change)  (step 1)
* Keep synchronized with the linkhandler
    - Let linkhandler know how many people are in every group (step 3)
    - If the partner choice matrix is updated the linkhandler needs to know
      (step 1)
    - Find which links are to be scheduled (step 4)
    - Let the linkhandler know when certain links are scheduled (step 4)
    - Change the sorting order of linkhandler. The linkhandler allows for
      manual sorting order (which allows for much less frequent sorting).
* Keep synchronized for statistics. Store info
    - Store #dropped persons due to there being to many (underscheduling of
      people from this group by the linkhandler, might happen if the relation
      ship matrix is off) (step 2)
    - Store #dropped persons due to not being able to find a suitable partner
      in 2 days. (might happen due to chance, strict partner-finding (small
      groups ranges) or due to a small number of relations to be scheduled.
    - Store #relations scheduled. (step 5)
* Keep track of which relationship request have priority and which don't. At
  the moment every relation which isn't scheduled on the exact same day as
  it was added is considered to be priority. (step 4a/4b). r_ is the
  relation request list r_p_ is the relation request priority list.
    - Move people from non-priority to priority groups
    - Add people to non-priority groups
* Check for robustness (step 2)
* Find matches (step 4)
    
OUTPUT: A vector containing all relation_requests to be scheduled. 

TERMINOLOGY
Link: A possible relation between 2 people
PreferenceMatrix: People from which groups prefer people from which groups?
  (odds. So in the 15-20 group 60% might prefer someone from 15-20 and 40%
  might prefer someone from the 20-25 group).
PartnerChoiceMatrix: Not everyone gets their preference. The partnerchoice
  matrix compensates for this. PCMatrix[i][j] gives the P(i gets a relation
  with j | i gets a relation).
*/

#ifndef SOA1_RG_MM_MATCHMAKER_H
#define SOA1_RG_MM_MATCHMAKER_H

#include <utility> // For pair (used to identify relation request) and move
#include <iostream> // For debugging
#include <random>   // For getting a random request (so the order of adding does not matter)

#include "soa1_rg_mm_robustness_check.h"
#include "soa1_rg_mm_group_handler.h"
#include "soa1_rg_mm_link_handler.h"
#include "soa1_rg_mm_relation_request.h"
#include "soa1_rg_mm_partner_choice_matrix.h"

namespace soa1 { // soa is the dutch word for sti
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking
class MatchMaker {
public:

  void AddRelationRequest(RelationRequest rr) {
    ++n_requests_received_;
    int group = group_handler_.GroupNumber(rr);
    relation_request_by_group_[group].push_back(rr);
    ++r_[group]; //Add 1 to the counter of the number of people in every group.
  }

  std::vector<std::pair<RelationRequest,RelationRequest>> Get() {

    /*
    0. Shuffle the new arrivals.
    1. Update the percentages in groups and see if the partner choice matrix
       can be updated
    2. Robustnesscheck
    3. Add the new relations to the linkhandler
    4. Schedule
    5. Remove relation requests unscheduled for 2 days
    6. Prepare the lists for the next day

    */
    std::vector<std::pair<RelationRequest, RelationRequest>> return_vec;

    // 0. Shuffle the new arrivals. 
    // Use shuffle because it is really fast (certainly compared to picking a
    // random relation request every time.) Another advantage is that we need
    // to do it only once.
    for (std::vector<mm::RelationRequest>& group : relation_request_by_group_){
      std::shuffle(group.begin(), group.end(), rng_);
    }

    // 1. Update the percentages in groups and see if the partner choice matrix
    //    and see if the partner choice matrix can be updated
    partner_choice_matrix_.UpdateDatabase(r_);
    if (partner_choice_matrix_.IsNewMatrixAvailable() == true) {
      link_handler_.UpdatePartnerChoiceMatrix(partner_choice_matrix_.Get());
    }

    // 2. robustnesscheck: 
    // GOAL:
    // It may happen that for some reason a certain group of persons fails to 
    // find matches. This might be noticed when they aren't scheduled after 2
    // days, but this might occur really late in a simulation (really really).
    // We do need to know rather soon if a certain group fails to schedule 
    // because otherwise the algorithm will start to do a lot of priority
    // scheduling which is bad for speed and accuracy.

    // If the robustnesscheck fails real sparingly (<1%) it is no problem 
    // (stochasticity), but usually it is something you want to look into.
    // Possible reasons for failure might be i) wrong implied group sizes 
    // resulting in wrong partner_choice_matrix or ii) the LinkHandler for
    // some reason fails to schedule the right links. 

    std::vector<int> remove_advice = 
      robustness_check_.UpdateandAdvice(r_, r_p_);

    // Loop over every group and remove the required number
    link_handler_.SortByLinks();
    for (int i = 0; i < group_handler_.NumberOfGroups(); ++i) {
      // Delete the adviced amount of groups. The RobustnessCheck class ensures
      // the adviced number is available.
      // Required to remove and needed for step 3 anyway.
      for (int to_remove = 0; to_remove < remove_advice[i]; ++to_remove) {
        ++n_requests_dropped_after_first_day_;
        r_p_[i]--;
        relation_request_by_group_priority_[i].pop_back();
        link_handler_.RemovePerson(i);
      }
    }

    // 3. Add the new relation_requests to the linkhandler
    //link_handler_.SortByLinks(); // Done in step 2
    link_handler_.Add(r_);

    // 4. scheduling!   
    // 4a. priority scheduling
    link_handler_.SortByValue(); // Needed for getting matches from linkhandler
    link_handler_.PointToTop();  // Start with most needed matches.

    while (link_handler_.PointsToAcceptableLink()) {
      alje::MultiSetSize2<int> possible_match = link_handler_.Get();
      const auto g1 = possible_match.first;
      const auto g2 = possible_match.second;

      // It makes sense... really it does. Just remember r_[i] is the number
      // of relation requests in the normal list and r_p_[i] for the priority
      // list. We basically check if we can find at least 1 match from the
      // priority group (we're priority scheduling here!)
      if ( g1 == g2 && (r_p_[g1] > 1 || r_p_[g1] > 0 && r_[g1] > 0) 
        || g1 != g2 && r_p_[g1] > 0 && (r_p_[g2] > 0 || r_[g2] > 0)
        || g1 != g2 && r_p_[g2] > 0 && (r_p_[g1] > 0 || r_[g1] > 0)
      ) {
        return_vec.push_back(MatchFoundDoAllAndReturnPair(g1, g2));
      } else {
        link_handler_.Next();
      }
    }

    // 4b. non-priority scheduling
    link_handler_.PointToTop(); // Now loop over the list again.
    while (link_handler_.PointsToPositiveLink()) {
      alje::MultiSetSize2<int> possible_match = link_handler_.Get();
      const auto g1 = possible_match.first;
      const auto g2 = possible_match.second;

      // At this point we are sure we cannot schedule any people from the
      // priority group. This would have been found out above (so no need 
      // in checking.
      if ( g1 == g2 && r_[g1] > 1 
        || g1 != g2 && !(r_[g1] == 0 || r_[g2] == 0) 
      ) {
        // A match has been found!
        return_vec.push_back(MatchFoundDoAllAndReturnPair(g1, g2));
      } else { // This match will not work
        link_handler_.Next();
      }
    }// !while

    // 5. Check for relation_requests which have been unscheduled for 2 days.
    // Remove these.
     for (int group_nr = 0; group_nr < group_handler_.NumberOfGroups(); 
            ++group_nr) {
      for (mm::RelationRequest relation_request : 
              relation_request_by_group_priority_[group_nr]) {
        // We end up here real sparingly so that is why we do the sorting
        // in this loop (instead of before the loop). The sorting 
        // is needed in order to remove.
        link_handler_.SortByLinks(); 
        ++n_requests_dropped_after_second_day_;
        link_handler_.RemovePerson(group_nr);
      }
      relation_request_by_group_priority_[group_nr].clear();
      r_p_[group_nr] = 0;
    }

    // 6. Prepare the lists for the next day.
    //    Move everyone which has been unscheduled for the first day to the
    //    priority group. Use swap because this might be faster and the 
    //    priority group has just been emptied above.
    std::swap(relation_request_by_group_, relation_request_by_group_priority_);
    std::swap(r_, r_p_);

    return return_vec;
  }// !Get()
  

  std::string LogReport() {    
    std::string return_string = "Start of logreport of MatchMaker\n"
      "Total relation request received: " 
      + std::to_string(n_requests_received_) + "\n"
      + "Total number of relation requests dropped after day 2"
      + "(should be low %): "
      + std::to_string(n_requests_dropped_after_second_day_) + " \n"
      + "Total number of relation requests dropped after day 1 "
      + "(should be very low %): "
      + std::to_string(n_requests_dropped_after_first_day_) + " \n"
      + "Total number of relations scheduled on first day: "
      + std::to_string(n_requests_scheduled_first_day_) + "\n"
      + "Total number of relations scheduled on second day: "
      + std::to_string(n_requests_scheduled_second_day_) + "\n";

    return_string += "\nLinkhandler supports MatchMaker.\n" +
      link_handler_.LogReport();
    return_string += "\nPartnerChoiceMatrix supports MatchMaker.\n" +
      partner_choice_matrix_.LogReport();
    
    return return_string;
  }// !LogReport()

  MatchMaker(
    rg::mm::GroupHandler group_handler, 
    rg::mm::PartnerChoiceParameters pcm_par, std::uint32_t seed)
      : 
      group_handler_(group_handler), 
      partner_choice_matrix_(group_handler_.PreferenceMatrix(), pcm_par),
      link_handler_(group_handler_.PreferenceMatrix()),
      robustness_check_(group_handler.NumberOfGroups(),
          n_percent_unscheduled_on_day_1_bound),
      rng_(seed){

    // Allow storage for the right number of groups. The resize function
    // automatically uses the default constructor for new elements. Since
    // empty vectors are default constructible this will work.
    relation_request_by_group_priority_.resize(
        group_handler_.NumberOfGroups());
    relation_request_by_group_.resize(group_handler_.NumberOfGroups());

    for (int i = 0; i < group_handler_.NumberOfGroups(); ++i) {
      r_.push_back(0);
      r_p_.push_back(0);
    }
  }// !Constructor

private:
  // Needs to be on top (due to construction), see RobustnessCheck for details.
  double n_percent_unscheduled_on_day_1_bound = 0.8; 
  soa1::rg::mm::GroupHandler group_handler_;
  soa1::rg::mm::PartnerChoiceMatrix partner_choice_matrix_; 
  soa1::rg::mm::LinkHandler link_handler_; // see note at top.
  soa1::rg::mm::RobustnessCheck robustness_check_; //See get step 1.
  std::minstd_rand rng_; // See get() step 0

  int n_requests_received_ = 0; // For statistics, incremented at Add()
  int n_requests_scheduled_first_day_ = 0; // incremented in MatchFoundDoAll()
  int n_requests_scheduled_second_day_ = 0;// incremented in MatchFoundDoAll()
  int n_requests_dropped_after_first_day_ = 0;// Get() step 1
  int n_requests_dropped_after_second_day_ = 0; // incremented in Execute()
  // This vector stores all the requests. There is a priority version for
  // the requests which couldn't be scheduled the timestep before.
  // relation_request_by_group[group][any_request].first (gives id)
  // relation_request_by_group[group][any_request].second (gives duration)
  std::vector<std::vector<mm::RelationRequest>> relation_request_by_group_;
  std::vector<std::vector<mm::RelationRequest>> 
                                        relation_request_by_group_priority_;
  std::vector<int> r_; // The number of people in different groups
  std::vector<int> r_p_; // See r_ but then for priority groups.

  std::pair<RelationRequest,RelationRequest> 
      MatchFoundDoAllAndReturnPair(int group1, int group2) {
    // If a match has been found this function will take care of all necessary
    // updating: 
    // i) linkhandler, 
    // ii) this class' relation_request_by_group and r_p_ and such.
    // iii) setting up the relationship events + updating the statistics

    // At this point you can be sure that A match has been found otherwise
    // we would not have been called.

    RelationRequest return_request1;
    RelationRequest return_request2;

    // i)
    link_handler_.Remove(); // We assigned one so now we can remove.
                    
    // ii) Update this class. Removing a random valid person is doen in get() 
    //      step0.
    if (r_p_[group1] > 0) {
      return_request1 = relation_request_by_group_priority_[group1].back();
      relation_request_by_group_priority_[group1].pop_back();
      r_p_[group1]--; // Decrease the number of people in the group.
      ++n_requests_scheduled_second_day_;
    } else {
      return_request1 = relation_request_by_group_[group1].back();
      relation_request_by_group_[group1].pop_back();
      r_[group1]--; // Decrease the number of people in a group.
      ++n_requests_scheduled_first_day_;
    } 
    if (r_p_[group2] > 0) {
      return_request2 = relation_request_by_group_priority_[group2].back();
      relation_request_by_group_priority_[group2].pop_back();
      r_p_[group2]--; // Decrease  the number of people in a group.
      ++n_requests_scheduled_second_day_;
    } else {
      return_request2 = relation_request_by_group_[group2].back();
      relation_request_by_group_[group2].pop_back();
      r_[group2]--; // Decrease the number of people in a group.
      ++n_requests_scheduled_first_day_;
    }

    return std::make_pair(return_request1, return_request2);
    // iii)     
 //   std::cout << "A match between " << group_handler_.GroupName(group1) << 
 //     " & " << group_handler_.GroupName(group2) << std::endl;
  }//!function ScheduleAll
};//!class MatchMaker
}// !namespace mm
}// !namespace rg
}// !namespace soa1

#endif// !SOA1_RG_MM_MATCHMAKER_H