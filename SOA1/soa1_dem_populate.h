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
GOAL: Initialize a population (in the sv::state, based on the parameter_pack).

IMPLEMENTATION: A function consisting of 3 parts.
  1. Some checks
  2. Initializing random variable genarators for age and such based on 
     the parameter pack
  3. Generate the (homogenous) population

*/

#ifndef SOA1_DEM_POPULATE_H
#define SOA1_DEM_POPULATE_H

#include <assert.h>
#include <random>
#include <memory>

#include "alje_event.h"
#include "alje_event_manager.h"
#include "soa1_sv_state.h"
#include "soa1_sv_person.h"
#include "soa1_dem_add_death_event.h"
#include "soa1_rg_ev_start_relation.h"

namespace soa1{
namespace dem{ // dem -> Demographics

//When initializing demographics we initialize the person list.
void Populate(sv::State& state) {

  // First check if the person_list is empty
  assert(state.person_list().Size() == 0 && "Error in Births. Trying to "
    "initialize the person_list but it wasn't empty");
  assert(state.time() == 0 && "Error in Populate, the model was populated at"
    " a t != 0.");

  // Initialize variables for use lateron.
  int sexual_onset = static_cast<int>(
    state.parameter_pack().relation_generation_start.sexual_onset * 365);
  int sexual_stop = static_cast<int>(
    state.parameter_pack().relation_generation_start.sexual_stop * 365);
  int n_persons = state.parameter_pack().demographics.initial_population;
  std::default_random_engine rng(state.seed_generator().Get());
  std::uniform_int_distribution<>random_age(sexual_onset,sexual_stop);
    
  // Start with a homogenous population
  std::unique_ptr<sv::Person> uptr_person;
  for (int i = 0; i < n_persons; ++i){
    // Insert the person into the list. Note that since a Person cannot be 
    // copy-constructed we use a temporary/rvalue in order to invoke the 
    // move constructor.
    sv::Person person_to_insert(
          -random_age(rng),
          state.parameter_pack(),
          state.seed_generator(),
          state.process_x_generator()
        );
    int person_id = person_to_insert.id();
    int t_next_relation = person_to_insert.next_relation_time();

    while (t_next_relation <= state.time()){
      // If a person does not have any relations anymore next_relation_time()
      // will return an std::numeric_limits<int>::max().
      t_next_relation = person_to_insert.next_relation_time();
    }
    
    state.person_list().Insert(std::move(person_to_insert));
    AddDeathEvent(state, person_id);

    std::unique_ptr<alje::Event> first_relation = 
        std::make_unique<soa1::rg::ev::StartRelation>(
        state, person_id, t_next_relation);
    state.event_manager().Add(std::move(first_relation));
  }
} // !function InitialBirths
} // !namespace dem
} // !namespace soa1
#endif // !SOA1_DEM_MAIN_H