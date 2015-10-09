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
GOAL: This file contains the StatusVariables struct. This is a wrapper around
all the variables which determine the state of a system. We use a single struct
for this in order to make it easier to pass the state to different functions.

NOTE: The construction of this state class is not trivial as it sometimes uses
components of itself in the construction. This means that those parts already
have to be constructed or errors will occur. This will not result in errors if
I remember that everything is instantiated in order of declaration.
*/

#ifndef SOA1_SV_State_H
#define SOA1_SV_State_H

#include <vector>
#include <iostream> // For debugging

#include "alje_event_manager.h"
#include "alje_process_x_generator.h"
#include "alje_rng_seed_generator.h"
#include "soa1_sv_person_list.h"
#include "soa1_sv_relation_list.h"
#include "soa1_sv_priority_list.h"
#include "soa1_rg_dur_get_duration.h"
#include "soa1_rg_mm_group_handler.h"
#include "soa1_rg_mm_matchmaker.h"
#include "soa1_tr_transmission.h"

#include "soa1_parameters_pack.h"

namespace soa1{
namespace sv { // sv -> Status Variables
  
class State{
public:
  int& time(){return time_;}
  const int& time()const{return time_;}
  int& current_priority(){return current_priority_;}
  const int& current_priority() const {return current_priority_;}
  PersonList& person_list(){return person_list_;}
  const PersonList& person_list() const {return person_list_;}
  RelationList& relation_list(){return relation_list_;}
  const RelationList& relation_list() const {return relation_list_;}
  const soa1::parameters::ParameterPack& parameter_pack(){
    return parameter_pack_;
  }
  soa1::rg::dur::GetDuration& get_duration() {return get_duration_;}
  const soa1::rg::dur::GetDuration& get_duration() const {
    return get_duration_;
  }
  soa1::rg::mm::MatchMaker& matchmaker() {return matchmaker_;}
  const soa1::rg::mm::MatchMaker& matchmaker() const {return matchmaker_ ;}
  alje::EventManager& event_manager(){return event_manager_;}
  alje::RngSeedGenerator& seed_generator(){return seed_generator_;}
  std::minstd_rand& random_number_generator(){return random_number_generator_;}
  alje::ProcessXGenerator& process_x_generator(){return process_x_generator_;}
  tr::Transmission& transmission() {return transmission_;}

  // Delegate constructor
  State() : State(alje::RngSeedGenerator::get_system_time()) { 
  }

  State(int seed) :
    parameter_pack_(),// For some reason this seems necessary
    seed_generator_(seed),
    process_x_generator_(
      parameter_pack_.relation_generation_start.stat_process_average / 365.0,
      parameter_pack_.relation_generation_start.weight_average,
      parameter_pack_.relation_generation_start.weight_short_history,
      parameter_pack_.relation_generation_start.weight_long_history,
      parameter_pack_.relation_generation_start.short_decay_rate_days,
      parameter_pack_.relation_generation_start.long_decay_rate_days,
      parameter_pack_.relation_generation_start.stat_process_n_prearrivals,
      seed_generator_.Get()
    ),
    get_duration_(
      parameter_pack_.relation_generation_duration,
      seed_generator_
    ),
    random_number_generator_(seed_generator_.Get()),
    group_handler_(time_,person_list_,parameter_pack_.matchmaking),
    matchmaker_(group_handler_,pcm_par_,seed_generator_.Get()),
    transmission_(parameter_pack_.sexual_behavior, parameter_pack_.hiv,
        parameter_pack_.gonorrhea,
        seed_generator_.Get())
    {} 

  State(const State&) = delete;
  State& operator=(const State&) =delete;


private:
  int time_ = 0;
  int current_priority_ = 0;
  PersonList person_list_;
  RelationList relation_list_;
  const parameters::ParameterPack parameter_pack_;
  alje::EventManager event_manager_;
  alje::RngSeedGenerator seed_generator_;
  std::minstd_rand random_number_generator_;
  alje::ProcessXGenerator process_x_generator_;
  soa1::rg::dur::GetDuration get_duration_;
  soa1::rg::mm::GroupHandler group_handler_;
  soa1::rg::mm::PartnerChoiceParameters pcm_par_; // use default values.
  soa1::rg::mm::MatchMaker matchmaker_;
  soa1::tr::Transmission transmission_;
};


} // !namespace sv
} // !namespace soa1

#endif //  !SOA1_CF_STATUS_VARIABLES_H