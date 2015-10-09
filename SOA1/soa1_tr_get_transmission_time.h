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
GOAL: Get the transmission time based on a step-function describing the
non-homogenous poisson process describing the rate of infection (which 
is provided to this class in a layer).

INPUT: A layer describing the rates of the nonhomogenous poisson process.

IMPLEMENTATION:
* See layer for a description of the step_function.
* Draw a unit random exponential variable, call that value e
* Find out for which T it holds that int_{t=0}^T m(t) dt == e. Where m is the 
  rate as specified in the input layer. 
* Since layers are step functions we start by finding the last step for which
  int_{t=0}^T m(t) dt < e
* Then it is an easy interpolation of (e - int_{t=0}^T m(t) dt)/last_step rate.

OUTPUT: A time in days that transmission will occur. (where layer.x is the
origin). Or -1 if no transmission will occur.

NOTES: 
* The layer class uses x and y. In this case this can be considered to be
  t and rate (m(t)) respectively. 
* The results of this class has been checked using excel.
*/

#ifndef SOA1_TR_GET_TRANSMISSION_TIME_H
#define SOA1_TR_GET_TRANSMISSION_TIME_H
#include <random>
#include <cstdio>
#include "soa1_tr_layer.h"
#include "soa1_tr_layer_assert_correct.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission
class GetTransmissionTime {
public:
  inline double Get(tr::Layer layer) { 
    assert(layer::AssertCorrect(layer));

    // The value we want the sum_before_i to be equal to.
    double val_remaining = unit_exp_dist_(rng_);
    double t = 0; // The last step which has been checked.
    double rate = layer[0].y; // The last rate which has been checked.

    for (int i = 0; i < static_cast<int>(layer.size()); ++i) {
      // If the time of this step is smaller than 0 the only thing relevant
      // is that it might describe the rate at t = 0;
      if (layer[i].x <= 0) {
        rate = layer[i].y;
        continue;
      }

      // If it is a positive coordinate it means we have a surface area
      // between our last time checked and now. So subtract this.
      val_remaining = val_remaining - (layer[i].x - t) * rate;

      // We might have swallowed too much. (i.e. int_{t=0}^T m(t) dt > e)
      // if so, fix this and stop the loop because we have found our 
      // final step.
      if (val_remaining < 0) {
        val_remaining = val_remaining + (layer[i].x - t) * rate;
        break;
      }

      // If we found a new step and the remaining value is still positive
      // the transmission time will be beyond our previous step so we update
      // our previous values.
      t = layer[i].x;
      rate = layer[i].y;
    }// !for i

    // At this point we have found our final step and t is the point where
    // the final step starts and val_remaining is e - int_{t=0}^T m(t) dt.
    // Since it is a step function this can easily be solved as follows.

    if (rate == 0) { // Special case.
      return -1; // No transmission!
    } else {
      return val_remaining / rate + t;
    }
  }// !function Get()

  GetTransmissionTime(std::uint32_t seed) : rng_(seed), unit_exp_dist_(1.0) {}
  GetTransmissionTime(const GetTransmissionTime&) = delete;
  GetTransmissionTime& operator=(const GetTransmissionTime&) = delete;

private:
  std::minstd_rand rng_;
  std::exponential_distribution<> unit_exp_dist_;
};//!class GetTransmissioNTime
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_GET_TRANSMISSION_TIME_H