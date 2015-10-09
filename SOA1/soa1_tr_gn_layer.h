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
GOAL: Provide a layer which captures all Gonorrhea-specific effects. 

INPUT: A relation, the persons in this relation, the simulation time.

INPUT ON CONSTRUCTION: The gn specific parameter structure. Containing a
base rate 

IMPLEMENTATION: The probability of GN transmission is constant.

OUTPUT: A layer giving the probability of transmission per unprotected anal 
intercourse over time. return_layer[t=0] is the current simulation time.
*/
#ifndef SOA1_TR_GN_LAYER_H
#define SOA1_TR_GN_LAYER_H
#include <cassert>
#include "soa1_tr_layer.h"
#include "soa1_tr_layer_assert_correct.h"
#include "soa1_tr_layer_move_forward.h"
#include "soa1_parameters_pack.h"
#include "soa1_sv_person.h"
#include "soa1_sv_relation.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

class GNLayer {
public:
  tr::Layer Get(const sv::Relation& relation, 
      const sv::Person& infected_person, 
      const sv::Person& susceptible_person, int simulation_t) {
    assert(infected_person.gonorrhea_status().infected() == true);
    assert(susceptible_person.gonorrhea_status().infected() == false);
    assert(
      infected_person.gonorrhea_status().TSinceInfection(simulation_t) >= 0);
    return gn_layer_;
  }
  GNLayer(parameters::TransmissionGonorrhea parameters) {
    // Transmission only depends on the base_rate.
    gn_layer_.push_back({0,parameters.base_rate});
  }
private:
  // Infectivity since infection (t=0 gives time of infection).
  tr::Layer gn_layer_;
};//!class GNLayer
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_GN_LAYER_H