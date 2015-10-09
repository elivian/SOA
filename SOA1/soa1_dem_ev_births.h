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

#ifndef SOA1_DEM_EV_BIRTHS_H
#define SOA1_DEM_EV_BIRTHS_H
#include <cassert>
#include <memory> // For unique_ptr to the next Births event.
#include "alje_event.h"
#include "soa1_sv_priority_list.h"
#include "soa1_sv_state.h"
#include "soa1_dem_births.h"

namespace soa1 { // SOA is the dutch equivalent of STI
namespace dem {  // dem -> demographics
namespace ev {   // ev -> event

class Births : public alje::Event {
public:
  virtual void Execute() override {

    dem::Births(state_);

    // Schedule the next Birth event
    std::unique_ptr<Event> next_event =
      std::make_unique<ev::Births>(state_, this->kTimeDue + 1);
    state_.event_manager().Add(std::move(next_event));
  }
  virtual bool Notify(int track, int number, int extra_info) override {
    assert("ev_Births->Notify should not be called (but it is!).");
    return false;
  }

  Births(sv::State& state, int time_due) : state_(state),
    Event(time_due, sv::priority::BIRTHS, -1, -1) {
  }

private:
  sv::State& state_;
};//!class MatchMakingAndFinalize
}// !namespace ev
}// !namespace rg
}// !namespace soa1
#endif// !SOA1_DEM_EV_BIRTHS_H