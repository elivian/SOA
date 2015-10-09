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
GOAL: Being the trigger for people becoming infected with gonorrhea.

INPUT: depends

IMPLEMENTATION: calling infect_person to take care of all the infecting details

OUTPUT: depends

NOTE: 
* We cannot assume that events are still relevant so we have to check this.
  (this is caused because of time-independance, us not allowing ourselves to
  change a certain priority group of events which are being executed, which in
  turn is done to avoid self-modifying behavior. This might result in someone
  being infected twice on one day (but by a different person)). 
* This class is very similar to the update_hiv_transmission event. I recommend
  all changes being made here to be made there as well.
*/
#ifndef SOA1_TR_EV_GN_TRANSMISSION_H
#define SOA1_TR_EV_GN_TRANSMISSION_H
#include <cassert>
#include "alje_event.h"
#include "soa1_sv_state.h"
#include "soa1_sv_priority_list.h"
#include "soa1_sv_relation_event_codes.h"
#include "soa1_tr_gn_infect_person.h"

namespace soa1 {// SOA is the dutch equivalent of STI
namespace tr {  // tr -> transmission
namespace ev {  // ev -> event

class GNTransmission : public alje::Event {
public:

  virtual void Execute() override {
    // person_to_infect_id_ gets infected!
    sv::Person& person_to_infect = state_.person_list()[person_to_infect_id_];

    // This person might already have been infected by another relation 
    // (probably in in this timestep as this event would have been removed
    // otherwise)
    if (person_to_infect.gonorrhea_status().infected() == false) {
      tr::GNInfectPerson(state_, person_to_infect);
    }
  }

  virtual bool Notify(int track, int number, int extra_info) {
    assert(number == relation_id_ && "Error, this event has been called "
      "but with an incorrect relation_id.");
    // No transmission after the relation ends
    if (extra_info == sv::relation_event_codes::END_DUE_TO_DEATH) {
      return true;
    }

    // Sometimes a transmission event will be cancelled because the GN
    // transmission event is updated. In this case remove this event. But only
    // if the model is not currently executing gn transmission events (to avoid
    // transmission events resulting in removing themself).
    if (extra_info == sv::relation_event_codes::CANCEL_EV_GN_TRANSMISSION &&
        state_.current_priority() != sv::priority::GN_TRANSMISSION){
        return true; // Let the eventmanager know it can remove this event.
    }
    return false;
  }

  GNTransmission(sv::State& state, int relation_id, int person_to_infect_id, 
      int time_due) 
    : alje::Event(time_due, sv::priority::GN_TRANSMISSION, -1, relation_id),
      state_(state),
      relation_id_(relation_id),
      person_to_infect_id_(person_to_infect_id){
  }

private:
  sv::State& state_;
  int relation_id_;
  int person_to_infect_id_;
  
};//!class Transmission
}// !namespace ev
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_EV_GN_TRANSMISSION_H