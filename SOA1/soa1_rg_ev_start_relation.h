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

#ifndef SOA1_RG_EV_START_RELATION_H
#define SOA1_RG_EV_START_RELATION_H

#include <memory> // For unique_ptr to new relation_start events.
#include "alje_event.h"
#include "soa1_sv_state.h"
#include "soa1_sv_priority_list.h"
#include "soa1_sv_person_event_codes.h"
#include "soa1_rg_add_relation_request.h"


namespace soa1 {
namespace rg { // rg -> Relationship generation
namespace ev { // ev -> event

class StartRelation : public alje::Event{
public:
  StartRelation(sv::State& state, int person_id, int time) : 
      state_(state),
      person_id_(person_id), 
      Event(time, sv::priority::RELATION_START, person_id, -1){}

  virtual void Execute() override {
 
    // Schedule the next relation
    sv::Person& this_person = state_.person_list()[person_id_];
    int next_relation_time = this_person.next_relation_time();

    soa1::rg::AddRelationRequest(
        state_, person_id_, next_relation_time - state_.time());
    
    // This part is needed to make sure one can start multiple relations in one
    // day. 
    while (next_relation_time == state_.time()){
      // In case there are no more relations std::numeric_limits<int>::max 
      // will be returned. 
      next_relation_time = this_person.next_relation_time();
      soa1::rg::AddRelationRequest(
        state_, person_id_, next_relation_time - state_.time());
    }

    std::unique_ptr<alje::Event> new_relation_event = 
        std::make_unique<ev::StartRelation>(state_,person_id_,
        next_relation_time);

    state_.event_manager().Add(std::move(new_relation_event));
  
  }// !function Execute

  virtual bool Notify(int track, int number, int extra_info) override {
    if (extra_info == sv::person_event_codes::DEATH){
      return true; // Remove me from the event manager!
    } else {
      return false; // Proceed as usual
    }
  }

private:
  sv::State& state_;
  int person_id_;

};//!class StartRelation
}// !namespace ev
}// !namespace rg
}// !namespace soa1 
#endif //! SOA1_RG_EV_START_RELATION_H
