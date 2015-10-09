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
GOAL: Infect a person with HIV. This will usually be called by the hiv 
transmission event (tr::ev::HIVTransmission).

INPUT: The simulation state and the person to infect (both by reference).

IMPLEMENTATION: 
1. Change the persons HIV status (and related variables)
2. Find all relations of this person and get a new time for transmission for
   this relation by calling UpdateHIVTransmissionEvent.

OUTPUT: void (updated hiv status of a person)

NOTES: 
* Since UpdateHIVTransmissionEvent creates new ev::HIVTransmission
  and ev::HIVTransmission calls InfectPerson, we cannot include 
  UpdateHIVTransmission event. This is solved by using a forward declaration
  a couple of lines down.
* We use the FindRelationsGivenPersonID function of relationlist (instead of
  using FindRelationIDsGivenPersonID) which will give us temporary pointers.
  but since updateHIVStatus does not change relations (only HIV) this will
  work (and be faster since we save a lookup in the relationlist).
* This class is very similar to the gn_infect_person class. I recommend
  all changes being made here to be made there as well.
*/

#ifndef SOA1_TR_HIV_INFECT_PERSON_H
#define SOA1_TR_HIV_INFECT_PERSON_H
#include "soa1_sv_state.h"
namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

// Forward declaration to avoid circular loop
void UpdateHIVTransmissionEvent(soa1::sv::State&,const soa1::sv::Relation&);

void HIVInfectPerson(sv::State& state, sv::Person& person_to_infect) {
  person_to_infect.hiv_status().infected() = true;
  person_to_infect.hiv_status().t_infected() = state.time();

  auto relations_infected_person = state.relation_list().
    FindRelationsGivenPersonID(person_to_infect.id());

  for (const sv::Relation* r : relations_infected_person) {
    tr::UpdateHIVTransmissionEvent(state, *r);
  }
}// !function InfectPerson

}// !namespace tr
}// !namespace soa1

#endif// !SOA1_TR_HIV_INFECT_PERSON_H