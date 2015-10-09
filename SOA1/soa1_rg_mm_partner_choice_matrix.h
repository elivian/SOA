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
GOAL: The goal of this function is twofold.
i) Get the partner_choice matrix
ii)This requires us to keep track of how many people are in each group.

The partner_choice matrix RM[i,j] is a matrix which will give the probability
that given someone who starts a relationship from group i that person will 
start a relationship with someone from group j. 

This partner_choice_matrix is based on the preference matrix. The preference
matrix gives the probability that "given someone from group i, what is the
probability that my first preference for a relation is with someone from
group j?". 

But people do not always get what they want. For example, people from group
3 might be real popular might there might be only 1 person. This means a lot
of people cannot get their preference. This illustrates 2 things:
i) we need a smart way to get from the preference matrix to the RM
ii) we need to know how many people actually are in this group.

INPUT: The number of people in every group adding the population.

INPUT ON CONSTRUCTION: preference_matrix (see goal for details), some
parameters. Most noteworthy might be the msm_hack_enabled. Enabling this leads
to better results but can only be done for msm.

IMPLEMENTATION: 
UpdateDatabase: Using a historic exponential weighting keep track of the number
  of people in every group.
IsNewMatrixAvailable: Checks if there is a significant difference in the group
  proportions (if so -> return true). Can be used to avoid unnecessary
  recomputation of the partner choice matrix.
Get: constructs a new PartnerChoiceMatrix. The partner choice matrix has two
  constraints: 
  i) rows must sum to 1 (if someone gets a relation it must be with someone)
  ii) sum over all groups of proportion of people in group i * RM(i,j) must
  equal the group size of j for every j. This means that every person from
  group j can have only 1 relation.
  We find the partner choice matrix by repeatedly scaling rows and columns to
  match constraint i and ii respectively.
ProvideFinishingTouch: By very slightly altering the PartnerChoiceMatrix make
  this matrix much more stable (better fit with the actual group_sizes). 
  Exploits the fact that when simulating MSM people in a certain group can
  always have a relation with people from the same group. So when a certain
  group is underscheduled just add a some relations in-group.
OUTPUT: PartnerChoiceMatrix (see goal above for details). If the 
  preference matrix was already well defined the partner choice matrix will be
  (possibly approximately) equal to the preference matrix. 

*/

#ifndef SOA1_RG_MM_PARTNER_CHOICE_MATRIX_H
#define SOA1_RG_MM_PARTNER_CHOICE_MATRIX_H

#include <vector>
#include <numeric> // for std::accumulate
#include <cstdlib> // for std::abs
#include <cassert>
#include <algorithm> // for std::max
#include "alje_historic_exponential_weighting.h"

namespace soa1 { // soa is the dutch equivalent of sti.
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

//Otherwise it can become bothersome really quick.
struct PartnerChoiceParameters {
  double weight_new_database_update = 0.001;
  int n_relation_matrix_iterations = 50;
  double group_estimate_error_tolerance = 0.0001;
  bool enable_msm_hack = true;
};

class PartnerChoiceMatrix {
public:
  std::vector<std::vector<double>> Get() {
    // See top -> implementation for details.
    // Extra favour for the IsNewMatrixAvailable function: set the group
    // proportions used in this calculation. (Used there to decide if a
    // recalculation is in order).
    // Furthermore for statistics, store how often this function is called.
    percent_in_group_estimate_last_recalculation_ = percent_in_group_estimate_;
    ++n_get_called_;

    // Start the iterations with the preference matrix (but make a copy to keep
    // the original).
    std::vector<std::vector<double>> return_vec = preference_matrix_;

    for (int i = 0; i < n_relation_matrix_iterations_; ++i) {
      // Step 1. If everyone would get their preference this might not fit with
      //  the actual number of people in every group (everyone might love Joe,
      //  but if there is only 1 Joe this might not work).
      for (int column = 0; column < n_groups_; ++column) {
        double column_sum = 0;
        for (int row = 0; row < n_groups_; ++row) {
          column_sum += percent_in_group_estimate_[row] * 
              return_vec[row][column];
        }
        // The number of relations with someone from group column should
        // be equal with the number of people in column column.
        double rescale_by = 1;
        // If the column_sum = 0 and it should equal zero we aren't rescaling.
        // if statement needed to avoid infinity errors.
        if (column_sum != 0 && percent_in_group_estimate_[column] != 0) {
          rescale_by = percent_in_group_estimate_[column] / column_sum;
        }
        for (int row = 0; row < n_groups_; ++row) {
          return_vec[row][column] *= rescale_by;
        }
      }
      // Step 2 make sure the rows in every group sum to 1 (if you're having a
      //  relation, you have to have it with someone).
      for (int row = 0; row < n_groups_; ++row) {
        double row_sum = 0;
        for (int column = 0; column < n_groups_; ++column) {
          row_sum += return_vec[row][column];
        }
        double rescale_by = 1 / row_sum;
        for (int column = 0; column < n_groups_; ++column) {
          return_vec[row][column] *= rescale_by;
        }
      }
    } //!for (iterations)

    if (msm_hack_enabled_) {
      return ProvideFinishingTouch(std::move(return_vec));
    } else {
      return return_vec;
    }
  }

  bool IsNewMatrixAvailable() {
    // This function checks if a new PartnerChoiceMatrix is available. If so it
    // returns true.

    // The implementation is that it checks if a new matrix ->should<- be
    // be available.

    double highest_abs_difference = 0;

    for (int i=0; i < static_cast<int>(percent_in_group_estimate_.size());++i){
      double current_abs_difference = std::abs(
          percent_in_group_estimate_[i] -
          percent_in_group_estimate_last_recalculation_[i]);
      if (current_abs_difference > highest_abs_difference)
        highest_abs_difference = current_abs_difference;
    }
    if (highest_abs_difference > group_estimate_error_tolerance_)
      return true;
    return false;
  }

  void UpdateDatabase(std::vector<int> n_people_in_group_new) {
    ++n_database_updates_called_;
    // Uses weighting average. See alje_historical_exponential_weighting for
    // a little more information.

    double total_new = std::accumulate(n_people_in_group_new.begin(),
        n_people_in_group_new.end(), 0.0);
    
    // If we don't receive any people we do not update
    if (total_new == 0.0)
      return;


    double weight_new = alje_weight_.GetWeight(n_database_updates_called_);
    double weight_previous = 1 - weight_new;
    for (int i = 0; i < n_groups_; ++i) {
      double percent_in_group_new = 
          static_cast<double>(n_people_in_group_new[i]) / total_new;
      percent_in_group_estimate_[i] = weight_new * percent_in_group_new
          + weight_previous * percent_in_group_estimate_[i];
     }
  }

  std::string LogReport() {
    std::string return_string = "PartnerChoiceMatrix report: \n"
      "Groups updated: " + std::to_string(n_database_updates_called_) + " \n"
      + "PartnerMatrix updated " + std::to_string(n_get_called_) + "\n"
      + "PartnerMatrix updating is computationally complex and you want this "
      "to be low compared to group updates.\n";

    return return_string;
  }

  double PercentInGroupEstimate(int group_nr) {
    return percent_in_group_estimate_[group_nr];
  }
    
  PartnerChoiceMatrix(std::vector<std::vector<double>> preference_matrix,
    PartnerChoiceParameters pcm_par
    ) :
    preference_matrix_(preference_matrix),
    weight_new_database_update_(pcm_par.weight_new_database_update),
    n_relation_matrix_iterations_(pcm_par.n_relation_matrix_iterations),
    group_estimate_error_tolerance_(pcm_par.group_estimate_error_tolerance),
    n_groups_(static_cast<int>(preference_matrix.size())),
    alje_weight_(pcm_par.weight_new_database_update),
    msm_hack_enabled_(pcm_par.enable_msm_hack)
  {
    // Assertions
    for (int row = 0; row < n_groups_; ++row) {
      double sum_of_row = std::accumulate(
          preference_matrix_[row].begin(), preference_matrix_[row].end(), 0.0);
      assert(sum_of_row < 1.001 && sum_of_row > 0.999 && "Error in soa1_rg_"
        "mm_partner_choice_matrix->Constructor. A preference matrix for which "
        "the rows do not sum to 1 has been passed in.");
    }
    for (int column = 0; column < n_groups_; ++column) {
      double column_sum = 0;
      for (int row = 0; row < n_groups_; ++row) {
        column_sum += preference_matrix_[row][column];
      }
      assert(column_sum > 0 && "Error in soa1::rg::mm::PartnerChoiceMatrix "
        "a preference matrix has been passed in which one group/column has a "
        "sum of 0. Meaning persons in this group aren't preferred by anyone! "
        "That is a nasty situation, not just for persons in these group but "
        "for the algorithm as well (it cannot convert a preference matrix to "
        "a relationmatrix this way). Best check your input.");
    }
    // Some more initialization (do all 0);
    percent_in_group_estimate_.resize(n_groups_);
    percent_in_group_estimate_last_recalculation_.resize(n_groups_);

  }

  PartnerChoiceMatrix() = delete; // We need parameters 



private:
  std::vector<double> percent_in_group_estimate_;
  std::vector<double> percent_in_group_estimate_last_recalculation_;
  int n_database_updates_called_ = 0;          // UpdateDatabase()
  int n_get_called_ = 0;                       // Get() for statistics
  const std::vector<std::vector<double>> preference_matrix_;// See Get()
  const double weight_new_database_update_;    // See UpdateDatabase()
  const int n_relation_matrix_iterations_;     // See Get()
  const double group_estimate_error_tolerance_;// See IsNewMatrixAvailable()
  const alje::HistoricExponentialWeighting alje_weight_; // UpdateDatabase()
  const int n_groups_;
  bool msm_hack_enabled_ = false;             // see ProvidFinishingTouch()

  std::vector<std::vector<double>> ProvideFinishingTouch(
      std::vector<std::vector<double>>&& pcm) {

    // Polish the edges of the PartnerChoiceMatrix to ensure that exactly the
    // right number of people get scheduled. Only works for msm because this
    // algorithm uses the fact that msm can always have relations with people
    // from the same group.

    assert(msm_hack_enabled_ == true && "Error in soa1::rg::mm::PartnerChoice"
      "Matrix->ProvideFinishingTouch");

    // Turn the partnerchoicematrix intro a matrix[i,j] where i,j is the % of
    // total relations are between i and j. For convenience lateron we will
    // assume that i,j != j,i (so in fact for i!=j the proportion of total
    // number of relation between i and j is matrix[i,j]+matrix[j,i]
    std::vector<std::vector<double>> fullmatrix(n_groups_, 
        std::vector<double>(n_groups_, 0));
    

    // Loop over full_matrix and fill this based on pcm. Immediately calculate
    // the row sums for the next step.
    std::vector<double> row_total_div_by_group_size(n_groups_,0);
    for (int i = 0; i < n_groups_; ++i) {
      for (int j = 0; j < n_groups_; ++j) {
        if (i == j)
          fullmatrix[i][i] = percent_in_group_estimate_[i] * pcm[i][i];
        else {
          // We take a minimum here because if we don't things will crash
          // if a certain group has no members. (A group which has no members
          // can choose relations with anyone as this has no effect. But in 
          // the end there shouldn't be any relations in this group.
          fullmatrix[i][j] = std::min(percent_in_group_estimate_[i] * 
              pcm[i][j],percent_in_group_estimate_[j]  * pcm[j][i]);
        }
        row_total_div_by_group_size[i] += 
            fullmatrix[i][j] / percent_in_group_estimate_[i];
      }
      if (percent_in_group_estimate_[i] == 0) { // Weird case
        row_total_div_by_group_size[i] = 0; // Assume no overscheduling.
      }
    }

    // We want to multiply fullmatrix by a factor to make sure no group gets
    // overscheduled. Something is overscheduled if the % of relations in 
    // fullmatrix is bigger than the size of the group.
    // Note: this might not be necessary (when you look at the row_total_div
    // by_groups_size everything is smaller than 1 already. Might be a 
    // result of taking the min in one of the steps above?
    double factor = *std::max_element(row_total_div_by_group_size.begin(), 
        row_total_div_by_group_size.end());
    
    for (auto& row : fullmatrix) {
      for (auto& cell : row) {
        cell = cell / factor;
      }
    }
    
    // Now we add values on the diagonal to make sure every row adds to the
    // group_size. Since group sizes sum to one, and rows now sum to group
    // sizes the total matrix will now nicely sum to 1.
    for (int i = 0; i < n_groups_; ++i) {
      double toadd = percent_in_group_estimate_[i] -
        std::accumulate(fullmatrix[i].begin(), fullmatrix[i].end(), 0.0);
      fullmatrix[i][i] += toadd;
    }

    // Now finally we need to convert back to a partner_choice matrix.
    // (rows summing to 1)
    for (int i = 0; i < n_groups_; ++i) {
      double row_sum = std::accumulate(fullmatrix[i].begin(), fullmatrix[i].end(), 0.0);
      if (row_sum != 0) {
        for (int j = 0; j < n_groups_; ++j) {
          fullmatrix[i][j] = fullmatrix[i][j] / row_sum;
          double cell = fullmatrix[i][j]; 
          if (!(cell > -100 && cell < 1000)) {
            int asd = 5;
          }
        } // end of for
      } else { // if row_sum == 0 (this implies the group size = 0)
        // For proper form the rows will still need to sum to 1. So we just
        // add in-group relations.
        fullmatrix[i] = std::vector<double>(n_groups_, 0);
        fullmatrix[i][i] = 1;
      }
    }



    // Now the fullmatrix should return a "perfect" matrix in the sense that
    // rows sum to 1 and columns sum to the group sizes.
    return fullmatrix;
  }

};//!class PartnerChoiceMatrix
}// !namespace mm
}// !namespace rg
}// !namespace soa1

#endif //!PARTNER_CHOICE_MATRIX_H