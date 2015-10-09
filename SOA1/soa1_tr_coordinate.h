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

#ifndef SOA1_TR_COORDINATE_H
#define SOA1_TR_COORDINATE_H

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

template <typename T = double>
struct Coordinate {
  T x;
  T y;
};//!class Coordinate
}// !namespace tr
}// !namespace soa1

#endif// !SOA1_TR_COORDINATE_H