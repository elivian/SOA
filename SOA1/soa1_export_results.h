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

#ifndef SOA1_EXPORT_RESULTS_H
#define SOA1_EXPORT_RESULTS_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "soa1_sv_state.h"

namespace soa1{
std::string ExportResults(sv::State& state){
  
  std::string return_string = "Exportresults Logreport \n" ;
  std::vector<const sv::Person*> all_persons_ptr_vec 
      = state.person_list().GetAll();

  int n_hiv_positive = 0;
  int n_gn_positive = 0;
  long long total_age_hiv_positive = 0;
  long long total_age = 0;
  std::vector<int> n_partners_hist;
  n_partners_hist.resize(5000);
  
  for (const sv::Person* person_ptr : all_persons_ptr_vec) {
    if (person_ptr->hiv_status().infected() == true) {
      ++n_hiv_positive;
      total_age_hiv_positive += (state.time() - person_ptr->day_of_birth());
    }

    if (person_ptr->gonorrhea_status().infected() == true) {
      ++n_gn_positive;
    }

    total_age += (state.time() - person_ptr->day_of_birth());
    int n_partners = state.relation_list().FindRelationsGivenPersonID(
        person_ptr->id()).size();
    ++n_partners_hist[n_partners];
  }

  double average_age_hiv_pos = static_cast<double>(total_age_hiv_positive) /
    static_cast<double>(n_hiv_positive) /365;
  double average_age = static_cast<double>(total_age) /
    static_cast<double>(state.person_list().Size()) /365;

  return_string += "N_hiv_positive: " + std::to_string(n_hiv_positive) + "/"
    + std::to_string(all_persons_ptr_vec.size()) + " (" +
    std::to_string(
      100 * n_hiv_positive / static_cast<double>(all_persons_ptr_vec.size())
    ) + "%)\nN_gn_positive: " + std::to_string(n_gn_positive) + "/"
    + std::to_string(all_persons_ptr_vec.size()) + " (" + 
    std::to_string(
      100* n_gn_positive / static_cast<double>(all_persons_ptr_vec.size())
    ) + "%) \nAverage age (years) (hivpos/overall): " 
    + std::to_string(average_age_hiv_pos) + "/" + std::to_string(average_age)
    + "\n\n";

  //std::ofstream n_partners_out("n_partners_hist.txt");
  for (int i = 0; i < 10; ++i) {
    std::cout << i << "\t" << n_partners_hist[i] << " \n";
  }

  return return_string;
};//!function ExportResults
}// !namespace soa1
#endif