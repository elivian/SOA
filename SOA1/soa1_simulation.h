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
GOAL: This is the main control flow file. One could say this is the control
room of a simulation. The main class keeps track of the time, the persons,
the relations and what is executed when.
*/
#ifndef SOA1_SIMULATION_H
#define SOA1_SIMULATION_H
#include <memory> // For unique_ptr to matchmaking event
#include "alje_event.h"
#include "soa1_sv_state.h"
#include "soa1_dem_populate.h"
#include "soa1_dem_births.h"
#include "soa1_dem_ev_births.h"
#include "soa1_sv_person.h"
#include "soa1_rg_add_relation.h"
#include "soa1_export_results.h"
#include "soa1_rg_ev_matchmaking.h"


namespace soa1 {
class Simulation{
public:
  void Start(){

    // Create people
    dem::Populate(state_); 

    // Infect people
    for (int i = 0; i < 500; ++i) {
      tr::HIVInfectPerson(state_, state_.person_list()[i]);
    }
    for (int j = 500; j < 1000; ++j) {
      tr::GNInfectPerson(state_, state_.person_list()[j]);
    }

    std::unique_ptr<alje::Event> births_event =
      std::make_unique<dem::ev::Births>(state_, 0);
    state_.event_manager().Add(std::move(births_event));

    std::unique_ptr<alje::Event> mm_event =
        std::make_unique<rg::ev::MatchMaking>(state_, 0);
    state_.event_manager().Add(std::move(mm_event));
    
    std::cout << "Started with seed: " 
      << state_.seed_generator().Seed() << "\n\n";

    for (int t = 0; t < 365 * 680; ++t){
      // Manually change the time to enforce strict seperation between the
      // state and the executing code.
       
      for (int p = sv::priority::FIRST; p <= sv::priority::LAST; ++p) {
        state_.time() = t;
        state_.current_priority() = p;
        state_.event_manager().ExecuteAll(t, p);
      }
      
      /*
      state_.time() = t; 

      // DEATHS
      state_.event_manager().ExecuteAll(t,sv::priority::DEATH);
      // BIRTHS
      dem::Births(state_);
      // FIND PEOPLE WHO WANT RELATIONS
      state_.event_manager().ExecuteAll(t,sv::priority::RELATION_START);
      // MATCHMAKING AND FINALIZE (SCHEDULE THE REALATIONS)
      state_.event_manager().ExecuteAll(
          t, sv::priority::MATCHMAKING_AND_FINALIZE);
      // END RELATIONS
      state_.event_manager().ExecuteAll(t, sv::priority::RELATION_END);
      // SPREAD HIV
      state_.event_manager().ExecuteAll(t, sv::priority::HIV_TRANSMISSION);
      // SPREAD GONORRHEA
      state_.event_manager().ExecuteAll(t, sv::priority::GN_TRANSMISSION);
      // NATURAL CURE GONORRHEA
      state_.event_manager().ExecuteAll(t, sv::priority::GN_NATURAL_CURE);
      
      */

      if (t % 365 == 0) {
        std::cout << state_.matchmaker().LogReport();
        std::cout << soa1::ExportResults(state_);
      }
    }
    int temp_for_debug = 0;
  }// !function Start
  

  Simulation(int seed) : state_(seed) {}
  Simulation() : state_() {}

private:
  sv::State state_; //Everything which has anything to do with the state

};// !class Simulation
} // !namespace soa1
#endif