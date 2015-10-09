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
GOAL: This provides an abstraction for the matchmaking class. The matchmaking
class only needs to know something as a "RelationRequest" exists but doesn't
need to know the implementation. This class can be changed according to a model
and then the matchmaking class does not need to be changed.
*/
#ifndef SOA1_RG_MM_RELATION_REQUEST_H
#define SOA1_RG_MM_RELATION_REQUEST_H

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation
namespace mm {   // mm -> matchmaking

struct RelationRequest {
  int person_id;
  int duration_in_days;
};//!class RelationRequest

}// !namespace mm
}// !namespace rg
}// !namespace soa1
#endif// !SOA1_RG_MM_RELATION_REQUEST_H