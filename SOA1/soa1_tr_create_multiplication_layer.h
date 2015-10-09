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
GOAL: Combine any number of layers into a new layer by multiplying
the values of all the layers.

INPUT: A vector of layers.

IMPLEMENATION:
1. Find every change in one of the underlying functions.
2. For every change in the underlying functions we the multiple of the
   functions might also change. So at every change in underlying function
   compute the value for the output layer.

OUTPUT: A layer which is the product of the input layers

NOTE: The code isn't lightning fast but since this probably isn't a performance
critical part we aimed for readibility.
*/
#ifndef SOA1_TR_CREATE_MULTIPLICATION_LAYER_H
#define SOA1_TR_CREATE_MULTIPLICATION_LAYER_H
#include <vector>
#include <numeric>    // For numeric_limits.
#include <algorithm>  // For std::sort
#include "soa1_tr_layer.h"
#include "soa1_tr_layer_assert_correct.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission
namespace { // Unnamed namespace so can only be called from this file.
// This function helps by getting the multiple. It does require the indices
// variable which contains, for every input layer, the index of that container
// that should be accessed.
inline double GetMultiple(const std::vector<Layer>& input_layers,
  const std::vector<int>& indices) {
  double multiple = 1;
  for (int i = 0; i < static_cast<int>(input_layers.size()); ++i) {
    multiple *= input_layers[i][indices[i]].y;
  }
  return multiple;
}// !function GetMultiple
}// !unnamed namespace

Layer CreateMultiplicationLayer(const std::vector<Layer>& input_layers) {
  // Some basic checks for validity
  for (const Layer& l : input_layers) {
    assert(layer::AssertCorrect(l));
  }

  // Find the points at which something might change,
  double lowest_x_val = std::numeric_limits<double>::max(); 
  std::vector<double> t_change;
  for (const Layer& l : input_layers) {
    // For every layer the first coordinate doesn't change anything.
    // (see the layer class description for more info on this).
    if (l[0].x < lowest_x_val) lowest_x_val = l[0].x;

    // All the other coordinates DO change something.
    for (int i = 1; i < static_cast<int>(l.size()); ++i) {
      t_change.push_back(l[i].x);
    }
  }
  
  // Now put them in the order they will appear in in the return layer.
  // (layers need to be sorted by x value)
  std::sort(t_change.begin(),t_change.end());
  // Remove any doubles (waste of space).
  auto end_it = std::unique(t_change.begin(), t_change.end());
  t_change.resize(end_it - t_change.begin());
  
  // Prepare the vector for insertion of the coordinates
  tr::Layer return_layer;
  return_layer.reserve(t_change.size());

  // Now loop over the times in t_change and at every point calculate the value
  // We do need to keep track, for every layer, at which step it currently is.
  // We use at_index for this. So at_index[2] = 5 means that input_layer 3, 
  // at this point in time has rate index[2][5].y.
  std::vector<int> at_index(input_layers.size(), 0);
  // We use lowest_x_val as the leftmost point (see tr::Layer for more info).
  return_layer.push_back(Coordinate<>{lowest_x_val, GetMultiple(input_layers,at_index)});

  for (double t : t_change) {
    // For every layer check if we should move to the next coordinate (i.e.
    // if there is a change in that function).
    for (int i = 0; i < static_cast<int>(input_layers.size()); ++i) {
      // A change can only occur if there are still coordinates left.
      if (at_index[i] +1 < static_cast<int>(input_layers[i].size())) {
        if (input_layers[i][at_index[i] + 1].x == t) {
          ++at_index[i]; // Yes! This is a function which changes at time t.
        }
      }
    }
    // Now update the value for this time. 
    return_layer.push_back(Coordinate<>{t, GetMultiple(input_layers, at_index)});
  }
  return return_layer;
}// !function CreateMultiplicationLayer
}// !namespace tr
}// !namespace soa1
#endif// !CREATE_MULTIPLICATION_LAYER