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
GOAL: A link is a combination between 2 groups (e.g. group 0 and group 5). The
linkhandler provides a couple of (supposedly) very fast function to deal with
those links.

INPUT: Varies per function, see the function in question for more details

INPUT ON CONSTRUCTION: A transition matrix(i,j). This transitionmatrix gives
for a person i the probability that he/she gets a relation with someone from
group j.

IMPLEMENTATION: At the heart of the LinkHandler is the link_list. This is a 
container which for every link stores a value. !This value is: 
E[number of matches based on this link] - number of matches based on this
link HAS been formed.! So a higher value means the algorithm is low on these
type of links.

The Linkhandler has been optimized for speed in a couple of ways. 
1. It stores only unique links so a link between group 0,4 is the same as a 
  link between group 4 and 0. This is handled automatically (by std::set 
  mostly). 
2. Link between groups which have a zero-probability of happening are set to
  -infinity in the link_list ensuring that the do not occur. This is done
  in the constructor.
3. It has two custom sorting options which have to be called before you can 
  use certain functions. This saves a lot of in-between sorting You can sort 
  by link, allowing you to add and remove new people to the population (and 
  thus change E[number of matches based on this link]). You can also sort by 
  value. This allows you to access the links based on value using a in-house 
  iterator.

The in-house iterator can be set to the link with the highest value, and can
be incremented (Next). It can also return (Get), Remove and check what it is
pointing to (PointsToPositiveLink, PointsToPossibleLink). All these functions
will keep the link_list_ sorted. These functions are needed in the larger 
scheme of things (see mm_matchmaker for more details).

OUTPUT: Varies per function, see the function in question for more details.
*/
#ifndef SOA1_RG_MM_LINK_HANDLER_H
#define SOA1_RG_MM_LINK_HANDLER_H
#include <vector>
#include <utility>   // For pair
#include <algorithm> // For sorting
#include <cassert>
#include <limits>
#include <numeric>   // For std::accumulate to check the sum of the tr. matrix.
#include <string>    // For the logreport

#include "alje_multiset_size2.h" // For storing links.

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

enum class LinkListState {
  SORTED_BY_VALUE,
  SORTED_BY_LINKS,
  UNSORTED
};

class LinkHandler {
public:
  inline void Add(std::vector<int> n_people_per_group) {
    assert(n_people_per_group.size() == n_groups_ && "Error in soa1::rg::mm::"
      "LinkHandler->Add has been called with a vector which does not have the "
      "right number of groups.");
    assert(link_list_state_ == LinkListState::SORTED_BY_LINKS && "Error in "
      "soa1::rg::mm::LinkHandler->Add has been called but the internal vector "
      "has not been sorted by links. Make sure you call SortByLinks before "
      "calling the add function.");
    assert(partner_choice_matrix_.size() > 0 && "Error in soa1::rg::mm::"
      "LinkHandler->Add has been called but the partner_choice matrix has not "
      "been initialized.");
    
    // Loop over every group and use the transition_matrix to determine how
    // many links would occur on average with other groups.
    // PositionInLinkSorted is a fast way to access the correct index.
    
    // Since a link is always between 2 persons we need to take care when
    // updating the E[number of matches based on this link]'s. 
    // Consider 5  persons in group 0 who generally have 40% of their relations
    // in group 1 and 3 persons in group 1 who generally have 67% of their 
    // relations in group 0. Adding 5*0.4 + 3*0.67 will yield 4 expected 
    // relations to be added on link [0,1], this should be 2. Hence we multiply
    // this number by 0.5 to get tge correct amount.

    for (int group_from = 0; group_from < n_groups_; ++group_from) {
      for (int group_to = 0; group_to < n_groups_; ++group_to) {
        link_list_[PositionInLinkSorted(group_from, group_to)].second += 
            0.5 * n_people_per_group[group_from] 
            *  partner_choice_matrix_[group_from][group_to];
      }
    }

  }// !Add()
  
  inline void PointToTop() {
    // Set the pointer to the top of the list.
    current_position_ = 0;
  }// !PointToTop()
  
  inline alje::MultiSetSize2<int> Get() const {
    // Get what the pointer/iterator is currently pointing to
    return link_list_[current_position_].first;
  }// !Get()
   
  inline std::string LogReport() {
    // A report on how we are doing
    // No speed needed (since this report should not be requested often)

    double highest = std::numeric_limits<double>::lowest(); // Start really low
    double lowest = std::numeric_limits<double>::max();
    double total = 0;
    for (auto link : link_list_) {
      if (link.second != std::numeric_limits<double>::lowest()) {
        total += link.second;
      }
      if (link.second > highest)
        highest = link.second;
      if (link.second != std::numeric_limits<double>::lowest() && link.second < lowest)
        lowest = link.second;
    }
    std::string return_string;
    return_string = "Linkhandler report. Current status: \n"
      "Highest value: " + std::to_string(highest) + "\n"
      "Lowest value: " + std::to_string(lowest) + "\n"
      "Total value: " + std::to_string(total) + "\n"
      "Times sorted by links: " + std::to_string(n_sorts_by_links_called_) 
        + "\n"
      "Times sorted by value: " + std::to_string(n_sorts_by_value_called_)
        + "\n";
     return return_string;
  }// !LogReport()

  inline void Next() {
    // Move the pointer/iterator one down.
    ++current_position_;
  }

  inline bool PointsToAcceptableLink() const {
    // Return true if the pointer/iterator is pointing to a link which might
    // occur. std::numeric_limits<double>::lowest() is -infinity and is the way
    // this class stores "not possible".
    if (current_position_ < static_cast<int>(link_list_.size())) { 
      // We consider a link acceptable if it hasn't been 
      if (link_list_[current_position_].second > acceptable_treshhold_) {
        return true;
      }
    }
    return false;
  }// !PointsToAcceptableLink()

  inline bool PointsToPositiveLink() const {
    // Return true if the pointer/iterator is pointing to a link which you want
    // to occur (a value > 0), i.e. a link which currently has occured less
    // than you would have expected based on the added persons.
    if (current_position_ < static_cast<int>(link_list_.size())) {
      if (link_list_[current_position_].second > 0) {
        return true;
      }
    }
    return false;
  }

  inline void Remove() {
    assert(PointsToAcceptableLink() && "Error in soa1::rg::mm::LinkHandler->"
      "Remove was called when the current_position does not point to a link "
      "which can occur. And you cannot remove links which cannot occur.");
    assert(link_list_state_ == LinkListState::SORTED_BY_VALUE && "Error in "
      "soa1::rg::mm::LinkHandler->Remove was called but the link_list isn't "
      "sorted by value. Call SortByValues first in order to be able to use "
      "the remove function. Thank you for your patience.");
    link_list_[current_position_].second -= 1;  // This link has happened.

    // Now resort by using a partial bubble sorting algorithm. This is great
    // because i) it allows us to sort only the value which has changed ii) it
    // will loop only over a small part of the list. 
    // We do this by finding the "right" position of our changed value by
    // repeatedly checking if the value below "our changed value" is lower. If
    // so, swap our value with the value below.
    int our_val_position = current_position_;
    // First check if there is a value below to compare with.
    while (our_val_position + 1 < static_cast<int>(link_list_.size())) {
      // Now compare. If the value below is lower, terminate the while loop
      // because we are done.
      if (link_list_[our_val_position + 1].second <=
          link_list_[our_val_position].second) {
        break;
      }
      // Apparently the value below was higher, so we need to move our value
      // one down the list.
      std::swap(link_list_[our_val_position],link_list_[our_val_position + 1]);
      ++our_val_position;
    }

  }// Remove()

  inline void RemovePerson(int remove_a_person_from) {
    // Sometimes for stability purposes we might need to remove a person. We
    // then reverse the "add" process to maintain consistency.
    assert(link_list_state_ == LinkListState::SORTED_BY_LINKS && "Error in "
      "soa1::rg::mm::LinkHandler->RemovePerson has been called, but this only"
      "works if the LinkList is sorted by links");
    
    // 1 person = 0.5 relation = 0.5 link.
    for (int group_to = 0; group_to < n_groups_; ++group_to) {

      link_list_[PositionInLinkSorted(remove_a_person_from, group_to)].second 
        -= 0.5*partner_choice_matrix_[remove_a_person_from][group_to];
    }
  }
 
  inline void SortByLinks() {
    ++n_sorts_by_links_called_;
    // Sort based on the links (because sorting pairs will first sort according
    // to  first and then to second and the link numbers are unique AND default
    // sorting is ascending we do not need to provide a custom sort). 

    // After having sorted in this way you can rapidly access elements by group
    // number by using link_list_[PositionInLinkSorted(group_nr_1,group_nr_2)].

   std::sort(link_list_.begin(), link_list_.end()); // According to link nr
   link_list_state_ = LinkListState::SORTED_BY_LINKS;
  }
  
  inline void SortByValue() {
    ++n_sorts_by_value_called_;
    // Now return the sort to sorting by how much we want it (the second
    // element in the pair). Sort descending. 
    
    
    std::sort(link_list_.begin(), link_list_.end(),
      [](const std::pair<alje::MultiSetSize2<int>, double>& left, 
         const std::pair<alje::MultiSetSize2<int>, double>& right) {
      return left.second > right.second;
    });
    
    link_list_state_ = LinkListState::SORTED_BY_VALUE;
  }
  
  inline void UpdatePartnerChoiceMatrix(std::vector<std::vector<double>> p) {
    partner_choice_matrix_ = p;
  }

  LinkHandler(std::vector<std::vector<double>> preference_matrix) :
    n_groups_(preference_matrix.size()),
    partner_choice_matrix_(preference_matrix) {
    assert(partner_choice_matrix_.size() != 0 && "Error in soa1::rg::mm::"
      "LinkHandler->Constructor An empty transition matrix has been passed.");
    assert(partner_choice_matrix_.size() ==  partner_choice_matrix_[0].size() 
      && "Error in soa1::rg::mm::LinkHandler->Constructor transition matrix "
      "dimensions do not agree.");
    for (int i = 0; i < n_groups_; ++i) {
      double sum_of_row = std::accumulate(
        partner_choice_matrix_[i].begin(),
        partner_choice_matrix_[i].end(), 0.0);
      assert(sum_of_row < 1.0001 && sum_of_row > 0.9999 && "Error in "
        "soa1::rg::mm::LinkHandler->Constructor the transition probabilities "
        "do not sum to 1");
    }
 
    // Initialize the link_list for every unique link between group i and j.
    // (so (1,3) is the same link as (3,1). To realise this we use higher and 
    // lower here instead of i and j,
    for (int higher = 0; higher < 
        static_cast<int>(partner_choice_matrix_.size()); ++higher) {
      for (int lower = 0; lower <= higher; ++lower) {
        link_list_.emplace_back(
            std::make_pair(alje::MultiSetSize2<int>(lower,higher), 0));
        if (partner_choice_matrix_[higher][lower] == 0) {
          // If this link occurs with probability 0 set it's value to "-inf".
          // Note that if transition_matrix_[i][j] = 0 than necessarily also
          // does transition_matrix_[j][i], so we check one.
          link_list_.back().second = std::numeric_limits<double>::lowest();
        } 
      }
    }
    // No guarantees of sorting on construction.
  }// !Constructor

private:
  int n_sorts_by_value_called_ = 0;
  int n_sorts_by_links_called_ = 0;
  int current_position_ = 0; // Start pointer/iterator by pointing to the top.
  const int n_groups_;
  // We still allow for priority persons to be scheduled even if a certain link
  // has been scheduled 10 times more than you'd want. The value for this does
  // not really matter (more negative = more relaxt) as long as it is finite.
  // The only reason to have a treshhold is to be able to detect unstable
  // situations. (because with a treshhold it will eventually stop returning
  // certain links and the problem will not 'magically' be solved by this 
  // class)
  const int acceptable_treshhold_ = -50; //

  // transition_matrix_[i][j] gives the vector for the probabilities that 
  // someone from group i gets someone from group j. This is not necessarily 
  // equal to transition_matrix[j][i] because of differing number of people
  // that are in each group. 
  // (But n_people_in_group[i] * transition_matrix[i][j] should equal
  // n_people_in_group[j] * transition_matrix[j][i].
  std::vector<std::vector<double>> partner_choice_matrix_;
  
  // The most important vector. This vector stores all "links". See top for 
  // more information. 
  // Vector of [link, number of links of this type to schedule]
  std::vector<std::pair<alje::MultiSetSize2<int>,double>> link_list_; 

  LinkListState link_list_state_ = LinkListState::UNSORTED;
  
  inline int PositionInLinkSorted(int group1, int group2) {
    // "Predict" at which index a certain link will be. This can be done by
    // combing the knowledge of which sets exist (0.5n^2 + 0.5*n, half of the
    // matrix + the elements on the diagonal) and the way the standard library
    // sorts sets. The derivation of this formula is great fun and is left for
    // the reader.

    // Reason for this prediction formula is that I expect it to be much faster
    // than a log(group_size^2) algorithm. It's kind of a unique_unordered_map.

    // Make sure group 1 is smaller so group (1,3) exists but (3,1) does not.
    // This is required because std::set (in which the links are stored)
    // automatically sorts this way as well.
    if (group1 > group2) std::swap(group1, group2);
    return group1 * (2 * n_groups_ - group1 - 1) / 2 + group2;
  }// !PositionInLinkSorted(...)
};//!class LinkHandler
}// !namespace mm
}// !namespace rg
}// !namespace soa1
#endif //!SOA1_RG_MM_LINK_HANDLER_H