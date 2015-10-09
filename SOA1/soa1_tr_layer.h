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
GOAL: Provide an abstraction which makes it easy to store step functions which
will be used as the rate function for a non-homogenous poisson process' which
will be used to determine the time of transmission. Reason for step function
is that it makes life much easier (when compared to just any function). Reason
for the non-homogenous poisson process is that it makes life much easier (when
compared to discrete sex).

INTERPRETATION: consider the layer {{1,3},{2,6},{5,8}}. This means a rate of
t<=1: a rate of 3
1<t<=2: a rate of 3
2<t<=5: a rate of 8
5<t: a rate of 8.

So {x,y} can be seen as: "Starting after x the rate will by y". 

NOTES:
* The coordinate class uses x,y. In this model this will be t, rate.
* The very first coordinate also determines the rate until -infinity 
  (see example)
* A layer must be sorted according to the time ascending.
* A layer must have at least 1 point.
*/

#ifndef SOA1_TR_LAYER_H
#define SOA1_TR_LAYER_H

#include <vector>
#include "soa1_tr_coordinate.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission
typedef std::vector<tr::Coordinate<double>> Layer;
}// !namespace tr
}// !namespace soa1

#endif// !SOA1_TR_LAYER_H