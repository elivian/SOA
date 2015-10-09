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
GOAL: This function will calculate the number of births required AND update
the person_list with the new births.

IMPLEMENTATION: 
* The person list will only contain the sexually active
  population. So when someone gets 'born' into the population list they are
  15 years old (or whatever the sexual onset is). 
* At the moment this algorithm uses a non-stochastic (=deterministic)
  population size. This is done in order to avoid unpredictable behavior
  (sudden huge increases in population might make analysing the results harder)

INPUT: The system state (parameter pack and current population are used for
sure, other things might)

OUTPUT: void (it updates the person list in the state)
 */

#ifndef SOA1_DEM_BIRTHS_H
#define SOA1_DEM_BIRTHS_H

#include <cmath>

#include "soa1_sv_state.h"
#include "soa1_sv_person.h"
#include "soa1_dem_add_death_event.h"
#include "soa1_rg_ev_start_relation.h"

namespace soa1{
namespace dem{

// Birth is starting to be sexual active
void Births(sv::State& state){
  int current_population = state.person_list().Size();
  int current_required_population = 
    state.parameter_pack().demographics.initial_population;
  int required_n_births = current_required_population - current_population;

  for (int i = 0; i < required_n_births; ++i){
    int day_of_birth = state.time() - 
        static_cast<int>(
        state.parameter_pack().relation_generation_start.sexual_onset * 365);

    sv::Person person_to_insert(
      day_of_birth,
      state.parameter_pack(),
      state.seed_generator(),
      state.process_x_generator()
    );

    int person_id = person_to_insert.id();

    // Both times in the line below refer to system time as stored in state (not
    // person time).
    int t_first_relation = person_to_insert.next_relation_time();
    state.person_list().Insert(std::move(person_to_insert));
    soa1::dem::AddDeathEvent(state, person_id);
    
    std::unique_ptr<alje::Event> first_relation = 
      std::make_unique<rg::ev::StartRelation>(state,person_id,t_first_relation);
    state.event_manager().Add(std::move(first_relation));


  }

}// !function Births

}// !namespace soa1
}// !namespace dem
#endif// !SOA1_DEM_BIRTHS_H