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

#ifndef SOA1_RG_EV_MATCHMAKING_H
#define SOA1_RG_EV_MATCHMAKING_H
#include <cassert>
#include <memory> // For unique_ptr to the next MatchMaking event.
#include "alje_event.h"
#include "soa1_sv_priority_list.h"
#include "soa1_sv_state.h"
#include "soa1_rg_add_relation.h"

namespace soa1 { // SOA is the dutch equivalent of STI
namespace rg {   // rg -> relationship generation
namespace ev {   // ev -> event

class MatchMaking : public alje::Event {
public:
  virtual void Execute() override {
 
    // Match people for relations
    std::vector<std::pair<rg::mm::RelationRequest, rg::mm::RelationRequest>>
      new_relations = state_.matchmaker().Get();

    // Now schedule these relations
    for (auto& relation : new_relations) rg::AddRelation(state_, relation);

    // Schedule the next matchmaking event
    std::unique_ptr<Event> next_event = 
        std::make_unique<MatchMaking>(state_, this->kTimeDue + 1);
    state_.event_manager().Add(std::move(next_event));
  }
  virtual bool Notify(int track, int number, int extra_info) override {
    assert("MatchMakingAndFinalize should not be called (but it is!).");
    return false;
  }

  MatchMaking(sv::State& state, int time_due) : state_(state),
    Event(time_due, sv::priority::MATCHMAKING, -1, -1) {
  }

private:
  sv::State& state_;
};//!class MatchMaking
}// !namespace ev
}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_EV_MATCHMAKING_H