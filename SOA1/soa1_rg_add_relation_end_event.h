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

#ifndef SOA1_RG_ADD_RELATION_END_EVENT_H
#define SOA1_RG_ADD_RELATION_END_EVENT_H
#include <memory> // for unique_ptr to event
#include "soa1_sv_state.h"
#include "soa1_rg_ev_end_relation.h"

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation

void AddRelationEndEvent(soa1::sv::State& state, int relation_id, int t_due) {
  std::unique_ptr<alje::Event> ev = 
      std::make_unique<rg::ev::EndRelation>(state, relation_id, t_due);
  state.event_manager().Add(std::move(ev));

}
}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_ADD_RELATION_END_EVENT_H