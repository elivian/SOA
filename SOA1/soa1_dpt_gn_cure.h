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
GOAL: 
1. Update the state variable to cure a person of gonorrhea,
2. Remove all other cure events
3. Update Gonorrhea transmission

INPUT: state variable and person_id
IMPLEMENTATION: see below
OUTPUT: NA (updates the state variable)
*/

#ifndef SOA1_DPT_EV_CURE_H
#define SOA1_DPT_EV_CURE_H
#include <cassert>
#include "alje_event.h"
#include "soa1_sv_state.h"
#include "soa1_sv_person_event_codes.h"
#include "soa1_tr_update_gn_transmission_event.h"

// Forward declarations to avoid circular inclusion loop
namespace soa1 {namespace tr {// tr -> transmission
void UpdateGNTransmissionEvent(soa1::sv::State&, const soa1::sv::Relation&);
}}

namespace soa1 { // soa is the dutch equivalent of sti
namespace dpt {  // dpt -> disease progression and treatment

void Cure(sv::State& state, int person_id) {
  // Cure this person
  sv::person::GonorrheaStatus& gn_status_ref = 
      state.person_list()[person_id].gonorrhea_status();
  gn_status_ref.infected() = false;
  gn_status_ref.t_infected() = -1; // To make a clear error more likely.

  // Let everyone interested know this person is cured.
  state.event_manager().NotifyChannel1(person_id, 
      sv::person_event_codes::GN_CURED);

  // Update Gonorrhea transmission, also update HIV transmission because
  // having Gonorrhea increases the probability of HIV transmission.
  std::vector<const sv::Relation*>& all_relations = 
      state.relation_list().FindRelationsGivenPersonID(person_id);
  for (const sv::Relation* relation_ptr : all_relations) {
    tr::UpdateGNTransmissionEvent(state, *relation_ptr);
    tr::UpdateHIVTransmissionEvent(state, *relation_ptr);
  }
};//!function cure
}// !namespace dpt
}// !namespace soa1
#endif// !SOA1_DPT_EV_GN_NATURAL_CURE_H