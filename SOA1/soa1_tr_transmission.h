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
GOAL: Being THE class to include for the transmission module.

INPUT: A relation, the persons in this relation and the current simulation time
INPUT ON CONSTRUCTION: All the parameter sets relevant for the STI's the model
is simulating (at the moment Sexual Behavior, HIV and Gonorrhoe).

IMPLEMENTATION: Since all soa's have some parts in common (the sexual part)
this class has a general function (at the bottom) which combines the specifics
of an STI (HIV/Gonorrhoe) with the sexual behavior. Users can call
Get[putyourstihere]Transmission to get a random transmission time for the next
time of infection.

OUTPUT: A time until the next infection occurs or -1 if this infection will
never occur (so 5 means 5 days from now). 
*/

#ifndef SOA1_TR_TRANSMISSION_H
#define SOA1_TR_TRANSMISSION_H
#include <cstdint> // For uint32_t
#include <cassert>
#include "soa1_tr_layer.h"
#include "soa1_tr_layer_assert_correct.h"
#include "soa1_tr_hiv_layer.h"
#include "soa1_tr_gn_layer.h"
#include "soa1_tr_sexual_behavior_layer.h"
#include "soa1_tr_create_multiplication_layer.h"
#include "soa1_tr_get_transmission_time.h"

#include "soa1_parameters_pack.h" 
#include "soa1_sv_relation.h" 
#include "soa1_sv_person.h"

namespace soa1 { // Soa is the dutch equivalent of sti
namespace tr {   // tr -> transmission

class Transmission {
public:

  inline double GetHIVTransmission(const sv::Relation& relation,
    const sv::Person& infected_person, const sv:: Person& susceptible_person,
    int simulation_time) {

    tr::Layer hiv_layer = hiv_layer_.Get( relation, infected_person, 
      susceptible_person, simulation_time);

    return this->GetTransmissionGivenSTILayer(relation, infected_person,
      susceptible_person, simulation_time, std::move(hiv_layer));
  }

  inline double GetGNTransmission(const sv::Relation& relation, 
    const sv::Person& infected_person, const sv::Person& susceptible_person, 
    int simulation_time) {

    tr::Layer gn_layer = gn_layer_.Get(relation, infected_person,
      susceptible_person, simulation_time); 

    return this->GetTransmissionGivenSTILayer(relation, infected_person, 
      susceptible_person, simulation_time, std::move(gn_layer));
  }

  Transmission(parameters::TransmissionSexualBehavior sexual_behavior, 
    parameters::TransmissionHIV hiv_parameters, 
    parameters::TransmissionGonorrhea gn_parameters, std::uint32_t seed) 
    : sexual_behavior_layer_(sexual_behavior),
      hiv_layer_(hiv_parameters),
      gn_layer_(gn_parameters),
      transmission_time_(seed) {
  }

  Transmission() = delete; // We need parameters!

private:
  tr::SexualBehaviorLayer sexual_behavior_layer_;
  tr::HIVLayer hiv_layer_;
  tr::GNLayer gn_layer_;
  // Turns a layer into a time for us:
  tr::GetTransmissionTime transmission_time_;

  // Function:
  inline double GetTransmissionGivenSTILayer(const sv::Relation& relation,
    const sv::Person& infected_person, const sv::Person& susceptible_person,
    int simulation_time, Layer&& sti_layer) {
    assert(tr::layer::AssertCorrect(sti_layer));

    tr::Layer sb_layer = sexual_behavior_layer_.Get(
      relation, infected_person, susceptible_person, simulation_time);
    assert(tr::layer::AssertCorrect(sb_layer));

    std::vector<tr::Layer> sb_and_sti{
      std::move(sb_layer),std::move(sti_layer)
    };
    tr::Layer total_layer = tr::CreateMultiplicationLayer(sb_and_sti);
    assert(tr::layer::AssertCorrect(total_layer));

    return transmission_time_.Get(total_layer);
  }
};//!class Transmission
}// !namespace tr
}// !namespace soa1
#endif// !SOA1_TR_ADD_STI_TRANSMISSION_EVENTS_H