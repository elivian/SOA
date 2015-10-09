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
GOAL: Find a relation duration for a person and turn this into a request to
be scheduled by the matchmaker.

INPUT: A person who wants a relation and the time until that person's next
relation.

OUTPUT: -
EFFECT: Adds a relation_request to the matchmaker.
*/

#ifndef SOA1_RG_ADD_RELATION_REQUEST_H
#define SOA1_RG_ADD_RELATION_REQUEST_H
#include "soa1_sv_state.h"
#include "soa1_temp_export.h" // XXX for debugging

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation

void AddRelationRequest(soa1::sv::State& state, int person_id, 
    int interrelation_time) {

  // Call the soa1::rg::dur::GetDuration class.
  int duration_in_days = state.get_duration().Get(interrelation_time);
  soa1::rg::mm::RelationRequest rr;
  rr.person_id = person_id;
  rr.duration_in_days = duration_in_days;
  state.matchmaker().AddRelationRequest(rr);
}

}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_ADD_RELATION_REQUEST_H