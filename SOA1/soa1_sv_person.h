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
GOAL: This file contains the person class. This class stores all information
for a single person. (like hiv_status, id etc.)

Note this does note keep track of the relations a person has. This is only
stored in the relation_list class to avoid duplicate information (which would
result in tedious syncing).
*/
#ifndef SOA1_SV_PERSON_H
#define SOA1_SV_PERSON_H
#include <limits>
// For seeding the stationary process
#include "alje_rng_seed_generator.h" 
#include "soa1_rg_start_next_relation_person_time.h"
#include "soa1_parameters_pack.h"
#include "soa1_sv_person_hiv_status.h"
#include "soa1_sv_person_gonorrhea_status.h"
namespace soa1 {
namespace sv { // sv -> status variables
class Person {
public:   
  const int id() const {return id_;}
  const int day_of_birth() const{return day_of_birth_;}
  inline const soa1::sv::person::HIVStatus& hiv_status() const {
    return hiv_status_;
  }
  inline sv::person::HIVStatus& hiv_status() {
    return hiv_status_;
  }
  inline const sv::person::GonorrheaStatus& gonorrhea_status() const {
    return gonorrhea_status_;
  }
  inline sv::person::GonorrheaStatus& gonorrhea_status() {
    return gonorrhea_status_;
  }
  int next_relation_time() {
    int next_relation_person_time = next_relation_person_time_.Get();
    if (next_relation_person_time == std::numeric_limits<int>::max()) {
      return std::numeric_limits<int>::max();
    } else {
      // Person time is days since birth, so to get the an absolute simulation
      // time we add the day of birth.
      return next_relation_person_time + day_of_birth_;
    }
  } // !member function next_relation_time()

  Person(int day_of_birth_supplied,
        soa1::parameters::ParameterPack par_pack,
        alje::RngSeedGenerator& seed_gen) :
        next_relation_person_time_(par_pack.relation_generation_start, seed_gen){
    day_of_birth_ = day_of_birth_supplied;
  }

  Person(int day_of_birth_supplied,
    soa1::parameters::ParameterPack par_pack,
    alje::RngSeedGenerator& seed_gen,
    alje::ProcessXGenerator& process_x_gen) :
    next_relation_person_time_(par_pack.relation_generation_start, 
                               seed_gen, process_x_gen){
    day_of_birth_ = day_of_birth_supplied;
  }

  // Since (among others) ID's might need incrementing we disable all of the
  // default copy-things.
  Person() = delete; 
  Person(const Person&) = delete;
  Person(Person&& person_supplied) = default;
  Person& operator=(const Person&) = delete;
private:
  // This function must be instantiated here because it is used  to initialize
  // the const variable id
  static int get_unique_id(){
    static int id = -1; // -1 because we want the first ID to be 0;
    // If we reach the max, restart (all the low ID people will be dead)
    if (id == std::numeric_limits<decltype(id)>::max())
      id = -1;
    ++id;
    return id;
  }
  int day_of_birth_;
  sv::person::HIVStatus hiv_status_;
  sv::person::GonorrheaStatus gonorrhea_status_;
  int id_ = get_unique_id();
  rg::start::NextRelationPersonTime next_relation_person_time_;

}; // !class Person
} // !namespace sv
} // !namespace soa1

#endif //  !SOA1_CF_PERSON_H