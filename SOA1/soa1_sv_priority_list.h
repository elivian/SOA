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

// The enum will now "pollute"  the priority namespace.
// everything can now be accessed as for example soa1::sv::priority::DEATH
// XXX meer uitleg hierzo!

#ifndef SOA1_SV_PRIORITY_LIST_H
#define SOA1_SV_PRIORITY_LIST_H

namespace soa1{
namespace sv { // sv -> status variables
namespace priority{ 
  
enum PriorityList {
  DEATH, // By default should get #0
  BIRTHS,
  RELATION_START,
  MATCHMAKING,
  RELATION_END,
  HIV_TRANSMISSION,
  GN_TRANSMISSION,
  GN_NATURAL_CURE,
  FIRST = DEATH, // We store a first and last to allow us to loop over
  LAST = GN_NATURAL_CURE
}; //!enum PriorityList
} // !namespace priority
} // !namespace sv
} // !namespace soa1

#endif