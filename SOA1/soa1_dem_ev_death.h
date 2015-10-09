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

#ifndef SOA1_DEM_EV_DEATH_H
#define SOA1_DEM_EV_DEATH_H

#include <vector>
#include "soa1_sv_state.h"
#include "soa1_sv_priority_list.h"
#include "alje_event.h"
#include "soa1_sv_person_event_codes.h"
#include "soa1_sv_relation_event_codes.h"

namespace soa1{
namespace dem{
namespace ev{
class Death : public alje::Event {
public:
  Death(sv::State& state, int person_id, int time) : state_(state), 
    person_id_(person_id), Event(time, sv::priority::DEATH, -1, -1){}
  virtual void Execute() override{

    // 1. Let everyone interested know this person is dying (in step 3)
    state_.event_manager().NotifyChannel1(person_id_, 
                                          sv::person_event_codes::DEATH);

    // 2. For every relation of this person, notify all interested events that
    // this relation ends.
    std::vector<int> all_relations =
      state_.relation_list().FindRelationIDsGivenPersonID(person_id_);
    for (int relation_id : all_relations){
      state_.event_manager().NotifyChannel2(relation_id,
          sv::relation_event_codes::END_DUE_TO_DEATH);
    }

    // 3. Remove the person from the person_list. This is done as step 3 (and
    // not 1 or 2) so that events notified in step 1 or 2 can still use access
    // this person and exit cleanly.
    state_.person_list().Erase(person_id_);

  }
  virtual bool Notify(int track, int number, int extra_info) override {
    // This is just required because we have to override the event class. It
    // doesn't do anything.
    return false; 
  }
private:
  sv::State& state_;
  int person_id_;

};// !class Death
} // !namespace ev
} // !mamespace dem
} // !namespace soa1
#endif // !SOA1_DEM_EV_DEATH_H