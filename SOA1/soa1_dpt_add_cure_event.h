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
GOAL: A function which adds a gonorrea cure event. Automatically takes care
of a difference in symptomatic/asymptomatic cure.

INPUT: the system state and the person to infect

IMPLEMENTATION: first get the time, then create the event and finally add the
event to the event manager.

OUTPUT: N/A (updated eventmanager (GN cure event added) 
*/

#ifndef SOA1_DPT_ADD_CURE_EVENT_H
#define SOA1_DPT_ADD_CURE_EVENT_H
#include "soa1_sv_state.h"
#include "soa1_sv_person.h"
#include "soa1_dpt_ev_gn_natural_cure.h"

namespace soa1 { // SOA is the dutch equivalent of STI
namespace dpt {  // dpt -> disease progression and treatment

void AddCureEvent(sv::State& state, const sv::Person& person) {
  
  int time_until_natural_cure = -42; // Initialize to N/A value
  if (person.gonorrhea_status().symptomatic() == true) {
    time_until_natural_cure =
      state.parameter_pack().gonorrhea.natural_cure_symptomatic;
  } else {
    time_until_natural_cure =
      state.parameter_pack().gonorrhea.natural_cure_asymptomatic;
  }
  std::unique_ptr<alje::Event> cure_event_ptr =
    std::make_unique<dpt::ev::GNNaturalCure>(state, person.id(),
      state.time() + time_until_natural_cure);
  state.event_manager().Add(std::move(cure_event_ptr));
}// !function AddCureEvent
}// !namespace dpt
}// !namespace soa1
#endif // !SOA1_DPT_ADD_CURE_EVENT_H