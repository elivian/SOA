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
GOAL: Be the trigger for a natural cure of gonorrhea.
INPUT: -
IMPLEMENTATION: call the soa1::dpt::gn::cure function.
OUTPUT: NA (modifies a person in the state variable)
*/

#ifndef SOA1_DPT_EV_GN_NATURAL_CURE_H
#define SOA1_DPT_EV_GN_NATURAL_CURE_H
#include <cassert>
#include "alje_event.h"
#include "soa1_dpt_gn_cure.h"
#include "soa1_sv_state.h"
#include "soa1_sv_person_event_codes.h"

namespace soa1 { // soa is the dutch equivalent of sti
namespace dpt {  // dpt -> disease progression and treatment
namespace ev {   // ev -> event

class GNNaturalCure : public alje::Event {
public:
  virtual void Execute() override{
    assert(state_.person_list()[person_id_].gonorrhea_status().infected()
      == true && "Error we are trying to cure someone from Gonorrhea who does"
      "not have Gonorrhea");
    soa1::dpt::Cure(state_, person_id_);
  }

  virtual bool Notify(int track, int number, int extra_info) override {
    assert(track == 1 && "Error, the GNNaturalCure->Notify was called on a"
      "track on which it is not listening");
    assert(number == person_id_ && "Error GNNaturalcure->Notify was called"
      "with a person_id to which we are not supposed to listen");
  
    if ( extra_info == sv::person_event_codes::DEATH) {
        return true;
    }
    // For natural_cure person events we do not remove this event because we 
    // assume there is only 1 natural cure event so a Notify with extra_info==
    // natural_cure_event will always indirectly be caused by the execute
    // function of this same class (and this event will be removed there).
    return false;
  }

  GNNaturalCure(sv::State& state, int person_id, int time) :
    state_(state),
    person_id_(person_id),
    Event(time, sv::priority::GN_NATURAL_CURE, person_id, -1) {}

private:
  sv::State& state_;
  int person_id_;

};//!class GNNaturalCure
}// !namespace ev
}// !namespace dpt
}// !namespace soa1

#endif// !SOA1_DPT_EV_GN_NATURAL_CURE_H