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
1.Infect a person with Gonorrhea. This will usually be called by the gn 
  transmission event (tr::ev::GNTransmission). 
2.Schedule the possible ways a person can get cured of Gonorrhea

INPUT: The simulation state and the person to infect (both by reference).

IMPLEMENTATION: 
1. Change the persons GN status (and related variables). Sample a random bool
   variable to determine if it will be symptomatic or asymptomatic.
2. Find all relations of this person and get a new time for transmission for
   this relation by calling UpdateGNTransmissionEvent.
3. Add natural cure
4. Recalculate the HIV transmission time (changes due to gn)

OUTPUT: void (updated GB status of a person)

NOTES: 
* Since UpdateGNTransmissionEvent creates new ev::GNTransmission
  and ev::GNTransmission calls InfectPerson, we cannot include 
  UpdateGNTransmission event. This is solved by using a forward declaration
  a couple of lines down.
* We use the FindRelationsGivenPersonID function of relationlist (instead of
  using FindRelationIDsGivenPersonID) which will give us temporary pointers.
  but since updateGNStatus does not change relations (only GN) this will
  work (and be faster since we save a lookup in the relationlist).
* This class is very similar to the hiv_infect_person class. I recommend
  all changes being made here to be made there as well.
*/

#ifndef SOA1_TR_GN_INFECT_PERSON_H
#define SOA1_TR_GN_INFECT_PERSON_H
#include <random> // For determining if it is asymptomatic or not
#include <memory> // For unique_ptr to cure event
#include "soa1_sv_state.h"
#include "soa1_dpt_add_cure_event.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

// Forward declarations to avoid circular inclusion loop
void UpdateGNTransmissionEvent(soa1::sv::State&, const soa1::sv::Relation&);

void GNInfectPerson(sv::State& state, sv::Person& person_to_infect) {
  
  // Change person infected_status
  const std::bernoulli_distribution random_is_symptomatic(
    state.parameter_pack().gonorrhea.probability_symptomatic);
  person_to_infect.gonorrhea_status().infected() = true;
  person_to_infect.gonorrhea_status().t_infected() = state.time();
  person_to_infect.gonorrhea_status().symptomatic() =
      random_is_symptomatic(state.random_number_generator());
  
  // Update all relations associated with this person
  auto relations_infected_person = state.relation_list().
      FindRelationsGivenPersonID(person_to_infect.id());
  for (const sv::Relation* r : relations_infected_person) {
    tr::UpdateGNTransmissionEvent(state, *r);
    tr::UpdateHIVTransmissionEvent(state, *r);
  }
 
  // Set natural cure for the person who was just infected.
  dpt::AddCureEvent(state, person_to_infect);

}// !function InfectPerson
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_GN_INFECT_PERSON_H