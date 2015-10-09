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
GOAL: Provide a single function which can be called if for some reason we want
to update the transmission of gonorrhea in a relation. 

INPUT: The state,  The relation for which to update and the reason for which
to update. This reason is in some cases needed to determine the exact control
flow. (reasons can be found in soa1::sv::relation_event_codes).

IMPLEMENTATION: 
1. Cancel all/the previous relation for this relation.
2. Schedule a new event.

OUTPUT: void (updated event_list)

NOTES:
* Since we are prescheduling there is no loss of accuracy if we just cancel the
  previous event and calculate a new event.
* Cancelling all/the previous relation will only work for timesteps which are
  not currently being excecuted (in order to prevent weird loops, e.g. events
  removing themselves before they have been fully executed).
* This class is very similar to the update_hiv_transmission event. I recommend
  all changes being made here to be made there as well.
*/

#ifndef SOA1_TR_UPDATE_GN_TRANSMISSION_EVENT_H
#define SOA1_TR_UPDATE_GN_TRANSMISSION_EVENT_H
#include <memory> // For unique_ptr to event
#include <cassert>
#include "alje_event.h"
#include "soa1_sv_state.h"
#include "soa1_sv_relation_event_codes.h" // For notifying channel 2.
#include "soa1_sv_person.h"
#include "soa1_sv_relation.h"
#include "soa1_tr_ev_gn_transmission.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

inline void UpdateGNTransmissionEvent(sv::State& state, 
    const sv::Relation& relation) {

  // Remove previous transmission events
  state.event_manager().NotifyChannel2(relation.relation_id(),
        sv::relation_event_codes::CANCEL_EV_GN_TRANSMISSION);

  const sv::Person& person1 = state.person_list()[relation.person1_id()];
  const sv::Person& person2 = state.person_list()[relation.person2_id()];

  double transmission_time_from_now = -1; 
  int person_to_infect_id = -42;  // Initialize to n/a value.
  // Serodiscordant?
  if (person1.gonorrhea_status().infected() 
      !=person2.gonorrhea_status().infected()){
    // Figure out which person is infected and get a random transmission time,
    // that function requires the first person to be infected, hence the "if".
    if (person1.gonorrhea_status().infected() == true) {
      transmission_time_from_now = state.transmission().GetGNTransmission(
          relation, person1, person2, state.time());
      person_to_infect_id = person2.id();
    } else {
      transmission_time_from_now = state.transmission().GetGNTransmission(
        relation, person2, person1, state.time());
      person_to_infect_id = person1.id();
    }
  } else {  // Serocordant ->
    return; // No Gonorrhea transmission.
  }

  if (transmission_time_from_now == -1) {
    // Apparently the Transmission->GetGNTransmission() returned -1 (no
    // transmission). 
    return; // So no Gonorrhea transmission
  }

  if (transmission_time_from_now + state.time() > relation.time_end()) {
    return; // Too late, not interesting.
  }
  
  // When scheduling transmission we need to know if it is still possible to
  // schedule for the current timestep (i.e. transmission events will still
  // be executed this timestep). If this is not the case we set the next 
  // timestep to be 0.
  int just_infected_mod = 0;
  if (state.current_priority() >= sv::priority::GN_TRANSMISSION)
    just_infected_mod = 1;
  // Static_cast rounds down.
  int transmission_simulation_time = state.time() +  
      static_cast<int>(transmission_time_from_now) +  just_infected_mod;

  assert(transmission_simulation_time >= state.time() && "Error a gonorrhea"
    "transmission event has been scheduled back in time");

  std::unique_ptr<alje::Event> new_e = 
      std::make_unique<tr::ev::GNTransmission>(
      state, relation.relation_id(), person_to_infect_id, 
      transmission_simulation_time);
 
  state.event_manager().Add(std::move(new_e));
 
}// function UpdateGNTransmissionEvent
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_UPDATE_GN_TRANSMISSION_EVENT_H