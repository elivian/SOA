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
GOAL: Provide a layer which captures all hiv-specific effects. 

INPUT: A relation, the persons in this relation, the simulation time.

INPUT ON CONSTRUCTION: The hiv specific parameter structure. Containing a
base rate and an infectivity in time since infection.

IMPLEMENTATION: The probability of HIV transmission varies depending on the 
stage (primary/acute, chronic, aids) a person is in. This stage at the moment
is determined deterministically depending only on the time since infection.

OUTPUT: A layer giving the probability of transmission per unprotected anal 
intercourse over time. return_layer[t=0] is the current simulation time.
*/
#ifndef SOA1_TR_HIV_LAYER_H
#define SOA1_TR_HIV_LAYER_H
#include <cassert>
#include "soa1_tr_layer.h"
#include "soa1_tr_layer_assert_correct.h"
#include "soa1_tr_layer_move_forward.h"
#include "soa1_parameters_pack.h"
#include "soa1_sv_person.h"
#include "soa1_sv_relation.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

class HIVLayer {
public:
  tr::Layer Get(const sv::Relation& relation, 
      const sv::Person& infected_person, 
      const sv::Person& susceptible_person, int simulation_t) {
    assert(infected_person.hiv_status().infected() == true);
    assert(susceptible_person.hiv_status().infected() == false);
    assert(infected_person.hiv_status().TSinceInfection(simulation_t) >= 0);
    
    tr::Layer return_layer(hiv_base_layer_);   
    // Increase the transmission probability depending on people having
    // gonorrhea.
    double total_multiplier = 1;
    if (infected_person.gonorrhea_status().infected() == true)
      total_multiplier *= hiv_pos_has_gn_multiplier_;
    if (susceptible_person.gonorrhea_status().infected() == true)
      total_multiplier *= hiv_pos_has_gn_multiplier_;
    for (auto& point : return_layer) {
      point.y *= total_multiplier;
    }

    int t_since_inf=infected_person.hiv_status().TSinceInfection(simulation_t);
    // Now that we now how long ago this person has been infected we can 
    // combine this with the general pattern of infectivity. If the person has
    // been infected for 50 days we should shift this pattern back in time by
    // 50 days to get the pattern for the person who has been infected for 50
    // days (hence the * -1)
    tr::layer::MoveForward(return_layer, -1 * t_since_inf); 
    return return_layer;
  }
  HIVLayer(parameters::TransmissionHIV parameters) :
    hiv_pos_has_gn_multiplier_(parameters.hiv_pos_has_gonorrhea_multiplier),
    hiv_neg_has_gn_multiplier_(parameters.hiv_neg_has_gonorrhea_multiplier) {
    for (const std::vector<double>& period : parameters.infectivity_over_time){
      hiv_base_layer_.push_back({period[0], period[1] * parameters.base_rate});
    }
  }
private:
  // Infectivity since infection (t=0 gives time of infection).
  tr::Layer hiv_base_layer_;
  double hiv_pos_has_gn_multiplier_;
  double hiv_neg_has_gn_multiplier_;


};//!class HIVLayer
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_HIV_LAYER_H