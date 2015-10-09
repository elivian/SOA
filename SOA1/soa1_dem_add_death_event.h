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
GOAL: A function to add a death event. The most important part is the
calculation of when the death is going to happen. At the moment this is still
at a fixed age but this might be improved in the future.
*/
#ifndef SOA1_DEM_ADD_DEATH_EVENT_H
#define SOA1_DEM_ADD_DEATH_EVENT_H

#include <memory> // For unique_ptr to death event.
#include "soa1_sv_state.h"
#include "soa1_dem_ev_death.h"

namespace soa1{
namespace dem{
void AddDeathEvent(sv::State& state, int person_id){

  // Calculate the day of death.
  // People die at their sexual_stop defined in the parameter pack (converted
  // to days because the model uses days).
  int day_of_birth = state.person_list()[person_id].day_of_birth();
  int sexual_stop_days_since_birth = static_cast<int>(
      state.parameter_pack().relation_generation_start.sexual_stop * 365
  );
  int day_of_death = day_of_birth + sexual_stop_days_since_birth;
  
  //Add the event
  std::unique_ptr<alje::Event> event_death = 
      std::make_unique<ev::Death>(state, person_id, day_of_death);
  state.event_manager().Add(std::move(event_death));
} // !function AddDeathEvent
} // !namespace dem
} // !namespace soa1
#endif // !SOA1_DEM_ADD_DEATH_EVENT_H