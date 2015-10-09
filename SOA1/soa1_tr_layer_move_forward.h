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
GOAL: Moves a layer forward in time. 

EXAMPLE: A layer might have been a step function which is 0 up until time 2
and 1 from time 2 until time 7. After calling MoveForward(layer, 3) it will 
now be 0 up until time 5 and be 1 from time 5 until time 10.

NOTES: 
* You can also move backwards in time.
* Remember: since you are working with doubles it might not be true that
  MoveForward(Moveforward(layer,3),-3) will return exactly the original.
*/
#ifndef SOA1_TR_LAYER_MOVE_FORWARD_H
#define SOA1_TR_LAYER_MOVE_FORWARD_H

#include <vector>
#include "soa1_tr_layer.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission
namespace layer {

inline void MoveForward(Layer& layer, double amount) {
  for (auto& c : layer) {
    c.x += amount;
  }
}

}// !namespace layer
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_LAYER_H