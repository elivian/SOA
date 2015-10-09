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
GOAL: The matchmaker has detemined which 2 persons will get a relation. Now
update everything for this relation.

*/

#ifndef SOA1_RG_ADD_RELATION_H
#define SOA1_RG_ADD_RELATION_H

#include "soa1_sv_state.h"
#include "soa1_rg_add_relation_end_event.h"
#include "soa1_tr_update_hiv_transmission_event.h"
#include "soa1_tr_update_gn_transmission_event.h"

#include "soa1_temp_export.h" //xxx for debugging

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation

inline void AddRelation(soa1::sv::State& state,
    std::pair<rg::mm::RelationRequest,rg::mm::RelationRequest> requests) {
  
  // Now everything is set and we really start the relation (it's official!)

  // But first plan when it stops
  // We randomly choose the relation_time between 1 of the 2 requests.
  int duration;
  const std::bernoulli_distribution pick_first_duration(0.5);
  if (pick_first_duration(state.random_number_generator())) {
    duration = requests.first.duration_in_days;
  } else {
    duration = requests.second.duration_in_days;
  }
  
  sv::Relation r(requests.first.person_id, requests.second.person_id,
    state.time(), state.time() + duration);

  // Store this relation in the system state
  state.relation_list().Insert(r);
  // Add events which can happen in a relation.
  rg::AddRelationEndEvent(state, r.relation_id(), state.time() + duration);
  tr::UpdateHIVTransmissionEvent(state, r);
  tr::UpdateGNTransmissionEvent(state, r);

  soa1::NewRelation(state, r.relation_id()); // For export

} // !function AddRelation()

}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_ADD_RELATION_H