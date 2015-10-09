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

#ifndef SOA1_RG_EV_END_RELATION_H
#define SOA1_RG_EV_END_RELATION_H

#include <memory> // For unique_ptr to new relation_end events.
#include <cassert>
#include "alje_event.h"
#include "soa1_sv_state.h"
#include "soa1_sv_priority_list.h"
#include "soa1_sv_relation_event_codes.h"
#include "soa1_rg_add_relation_request.h"


namespace soa1 {
namespace rg { // rg -> Relationship generation
namespace ev { // ev -> event

class EndRelation : public alje::Event {
public:
  EndRelation(sv::State& state, int relation_id, int time) :
    state_(state),
    relation_id_(relation_id),
    Event(time, sv::priority::RELATION_END, -1, relation_id) {}

  virtual void Execute() override {
    state_.relation_list().RemoveGivenRelationID(relation_id_); 
  }// !function Execute

  virtual bool Notify(int track, int number, int extra_info) override {
    // The track number will be 2
    assert(track == 2 && "Error in soa1_rg_ev_end_relation-> Notify was "
      "called on track 1 but we aren't even listening on track 1!");
    assert(number == relation_id_ && "Error in soa1_rg_ev_end_relation->Notify"
      "was called with a number which does not match the relation id. Weird.");

    if (extra_info == soa1::sv::relation_event_codes::END_DUE_TO_DEATH) {
      Execute(); // Premature execution, but that is OK
      // Since it is 'us' calling Execute (not the event manager) this event
      // will not autmoatically be removed, so we need to let the event manager
      // know we want to be removed.
      return true; // Yes remove us from the event list! 
    } 
    return false;
  }

private:
  sv::State& state_;
  int relation_id_;
};//!class EndRelation
}// !namespace ev
}// !namespace rg
}// !namespace soa1 
#endif //! SOA1_RG_EV_END_RELATION_H
