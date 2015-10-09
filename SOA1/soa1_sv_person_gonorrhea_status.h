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
GOAL: Provide a class which sotres everything about the Gonorrhea status
of a person.
*/

#ifndef SOA1_SV_PERSON_GONORRHEA_STATUS_H
#define SOA1_SV_PERSON_GONORRHEA_STATUS_H
#include <cassert>
#include <iostream>
namespace soa1 {
namespace sv {
namespace person {

class GonorrheaStatus {
public:

  inline bool& infected() {return infected_;}
  inline const  bool& infected() const { return infected_; }

  inline int& t_infected() {
    return t_infected_;
  }

  inline const int& t_infected() const {
    assert(infected_ == true && "Error, the time for Gonorrhea infection was "
      " requested but this person is not infected.");
    return t_infected_;
  }
  inline bool& symptomatic() {
    assert(infected_ == true && "Error, the symptmatic status for Gonorrhea" 
      "infection was requested but this person is not infected.");
    return symptomatic_;
  }

  inline const bool& symptomatic() const {
    assert(infected_ == true && "Error, the symptmatic status for Gonorrhea"
      "infection was requested but this person is not infected.");
    return symptomatic_;
  }

  inline int TSinceInfection(int t_current) const {
    assert(infected_ == true && "Error, the time since Gonorrhea infection was"
      " requested but this person is not infected.");
 
    return t_current - t_infected_;
  }
  inline void Infect(int time, bool symptomatic) {
    infected_ = true;
    t_infected_ = time;
    symptomatic_ = symptomatic;
  }
  inline void Cure() {
    infected_ = false;
    t_infected_ = -1;
  }
private:
  bool infected_ = false;
  int t_infected_ = -1;
  bool symptomatic_ = false;
};//!class GONORRHEAStatus
}// !namespace person
}// !namespace sv
}// !namespace soa1
#endif// !SOA1_SV_PERSON_GONORRHEA_STATUS_H
