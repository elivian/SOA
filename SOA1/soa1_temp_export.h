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

#ifndef TEMP_EXPORT_H
#define TEMP_EXPORT_H

#include <iostream>
#include <fstream> //For writing to a file
#include <vector>

#include "alje_pdf_vector_functions.h"
#include "soa1_sv_state.h"
#include "soa1_sv_person_event_codes.h"


namespace soa1 {

void PersonGetsRelation(int person_id,sv::State& state ){

  //The next section of code generates the total number of relationships 
  // received in the last half year. When there is a homogenous population
  // this can be easily used. :/
  static std::vector<long long> n_relations_given_age(81, 0);
  static int last_t_processed = 0;


  int person_age = (state.time() - state.person_list()[person_id].day_of_birth()) / 365;
  ++n_relations_given_age[person_age];

  if (state.time() % 183 == 0 && state.time() != last_t_processed) {
    std::ofstream out_file("n_relationstarts.txt");
    for (int i = 0; i < 81; ++i) {
      out_file << i << "\t" << n_relations_given_age[i] << "\n";
    }
    last_t_processed = state.time();
    std::vector<long long> temp(81, 0);
    n_relations_given_age = temp;
  }


  


  return; // xxx temporary disable

  

  /*
  // Part 0 total nr relations assuming no population increase
  int n_persons = state.parameter_pack().demographics.initial_population;
  int sexual_onset_years = static_cast<int>(state.parameter_pack().relation_generation_start.sexual_onset);
  int sexual_stop_years = static_cast<int>(state.parameter_pack().relation_generation_start.sexual_stop);
  int sexual_lifetime_years = static_cast<int>(sexual_stop_years - sexual_onset_years);

  static int total_relations = 0;
  ++total_relations;
  int lifetime_n_partners_estimate = static_cast<int>(
    (static_cast<double>(total_relations) * 365 * sexual_lifetime_years) / n_persons / state.time());
  
  
  if (state.time() % 365 == 0)
    std::cout << "Estimated total lifetime n partners = " << lifetime_n_partners_estimate << std::endl;
  
  */
  
  /*
  // Part 0b
  static int total_calls = 0;
  if (person_id == 10001) {
    ++total_calls;
    std::cout << "person 10001 has n_relations: " << total_calls << std::endl;
  }
 */

  
  /*
  // Part 0b
  static int total_relations_executed = 0;
  ++total_relations_executed;
  static int old_time = 0;
  if (old_time != state.time()){
    old_time = state.time();
    std::cout << "Total relations executed:" << total_relations_executed << std::endl;
  }
  */

  // part 0c
  /*
  // PART 1 Relations as a function of age
  static std::vector<int> total_n_partners_at_age(90,0); // 90 times a zero
  int person_age = (state.time() - state.person_list()[person_id].day_of_birth()) / 365;
  
  // Only include people who aren't preseeded (so who are born after the simulation has started)
  // if (state.person_list()[person_id].day_of_birth() > 0) {
    ++total_n_partners_at_age[person_age];
  // }

  if (state.time() % 3650 == 0) {
    std::ofstream out_file("total_n_partners_at_age.txt");
    for (int i = 0; i < static_cast<int>(total_n_partners_at_age.size()); ++i) {
      out_file << i << "\t" << static_cast<double>(total_n_partners_at_age[i]) << std::endl;
    }
  }

  // PART 2 #partners in last 6 months
  static std::vector<int> partners_in_last_6_months(1000000, 0); // 1million personID's that can be stored
  
  ++partners_in_last_6_months[person_id];

  static int previous_state_time = 0; // To ensure the following code only executes once
  if (state.time() % 183 == 0 && state.time() != previous_state_time) { // Every half year... output and reset
    previous_state_time = state.time();
    // First count the number of occurences
    std::vector<int> count_vec = alje::samples_vector_to_count_vector(partners_in_last_6_months, 1000);
    // Remove all people who have had 0 partners (because this also includes the persons unborn as of yet)
    count_vec[0] = 0; 
    // Turn it into a pdf
    std::vector<double> pdf_vec = alje::count_vector_to_pdf(count_vec);
    // Export it to a file
    alje::output_pdf_vec(pdf_vec, "partners_in_last_6_months_general.txt");
    //Reset
    std::fill(partners_in_last_6_months.begin(), partners_in_last_6_months.end(), 0);
    std::ofstream out_file2("partners_in_6_months");
  }

  // PART 3 do we have relation starts on the same day? 
  */
}


void NewRelation(sv::State& state, int relation_id) {
  
  // DISABLED WE DON'T WANT INFO. 
   return;

  static int n;
  ++n;
  static std::vector<int> relation_duration_list(5000,0);
  static std::vector<int> person1(5000,0);
  static std::vector<int> person2(5000,0);

  const sv::Relation* rel = state.relation_list().PointerGivenRelationID(relation_id);

  relation_duration_list[n % 5000] = rel->time_end() - state.time();
  person1[n % 5000] = (state.time() - state.person_list()[rel->person1_id()].day_of_birth()) / 365 ;
  person2[n % 5000] = (state.time() - state.person_list()[rel->person2_id()].day_of_birth()) / 365;
  

  static double avg_dur = 0;



  if (n%10000 == 0) {
    std::ofstream out("Relations.txt");
    out << "NEWRELATIONS: n=" << n << std::endl;
    out << "Duration \t Age1 \t age2" << std::endl;
    for (int i = 0; i < 5000; ++i) {
      out << relation_duration_list[i] << "\t" <<
        person1[i] << "\t" << person2[i] << std::endl;
      out.flush();
    }
    out.close();
    std::cout << "Updated" << std::endl;
  }



}

void RelationEnds(sv::State& state, int relation_id) {}


}// !namespace soa1
#endif // !TEMP_EXPORT_H