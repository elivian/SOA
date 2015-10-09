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

#ifndef SOA1_SV_RELATION_H
#define SOA1_SV_RELATION_H
#include <limits>

namespace soa1{
namespace sv{ // sv -> Status Variables

class Relation {
public:
  const int relation_id() const{return relation_id_;}
  const int person1_id() const{return person1_id_;}
  const int person2_id() const{return person2_id_;}
  const int time_start() const{return time_start_;}
  const int time_end() const{return time_end_;}
    
  Relation(int person1_id, int person2_id, int time_start, int time_end) :
    person1_id_(person1_id),
    person2_id_(person2_id),
    time_start_(time_start),
    time_end_(time_end){
  }

private:
  // The following function must be placed here because it is used to initialize
  // a constant variable (and therefore this function must be accessible).
  static int get_unique_id(){
    static int id = -1; // -1 because we want the first ID to be 0;

    // If we reach the max, restart (all the low ID people will be dead)
    if (id == std::numeric_limits<decltype(id)>::max())
      id = -1;

    ++id;
    return id;
  }

  const int relation_id_ = get_unique_id();
  const int person1_id_;
  const int person2_id_;
  const int time_start_;
  int time_end_; // Can change
  
}; // !class Relation
} // !namespace sv
} // !namespace soa1
#endif //  !SOA1_CF_RELATION_H