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
GOAL: Provide a layer which determines the rate of unprotected anal intercourse

INPUT: Relation, the persons in this relation and the current simulation time.
INPUT ON CONSTRUCTION: Sexual behavior parameters

IMPLEMENTATION: At the moment the rate of unprotected anal intercourse is
constant and depends only on the sex frequency (constant) and condom use 
(constant).

OUTPUT: A rate of unprotected anal intercourse which can vary over time (t=0
is the current simulation time).
*/
#ifndef SOA1_TR_SEXUAL_BEHAVIOR_LAYER_H
#define SOA1_TR_SEXUAL_BEHAVIOR_LAYER_H
#include "soa1_tr_layer.h"
#include "soa1_parameters_pack.h"
#include "soa1_sv_person.h"
#include "soa1_sv_relation.h" 

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

class SexualBehaviorLayer {
public:
  Layer Get(const sv::Relation& relation, const sv::Person& person1, 
    const sv::Person& person2, int simulation_t) {
    return sb_layer_;
  }

  SexualBehaviorLayer(parameters::TransmissionSexualBehavior parameters) {
    // unprotected anal intercourse rate
    double uai_rate = (1.0 - parameters.condom_use) * parameters.sex_frequency;
    // Create a layer with this constant rate.
    sb_layer_.push_back({0,uai_rate});
  }

private:
  tr::Layer sb_layer_;
};//!class SexualBehaviorLayer
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_SEXUAL_BEHAVIOR_LAYER_H