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
GOAL: A contianer which stores the all the persons. Persons can be accessed 
by ID.

IMPLEMENTATION: At the moment it uses an std::map, but this can be changed to 
an unordered map. But in preliminary tests the map showed to be faster (but
then again I read something online of the visual studio 2013 unordered map to 
be somewhat bugged.

INPUT: depends on the function called, but usually a person_id

OUTPUT: depends on the function called.
*/

#ifndef SOA1_SV_PERSON_LIST_H
#define SOA1_SV_PERSON_LIST_H

#include <unordered_map>
#include <map> //For debug, is this faster?

#include "soa1_sv_person.h"

namespace soa1{ // soa1 -> dutch word for STI
namespace sv{   // sv -> status variables

class PersonList{
public:
  void Insert(Person person){
    person_map_.insert(std::make_pair(person.id(), std::move(person)) );
  }
  
  void Erase(int personID){
    person_map_.erase(personID);
  }

  int Size() const{
    return person_map_.size();
  }

  Person& operator[](const int personID){
    // std::unordered_map::operator[] requires the object in the map to be
    // default constructible (because it will return a default constructed
    // object if it wasn't found. Hence we use .at() here because this will
    // throw an std::out_of_range exception if it doesn't exist (and thus
    // doesn't require a person to be default constructible).
    
    // For debugging:
    //if (person_map_.find(personID) == person_map_.end())
    //  int temp_for_debug = 0;
    
    return person_map_.at(personID); 
  }

  // Might be real slow but can be used to generate statistics about the 
  // population
  std::vector<const Person*> GetAll() const {
    std::vector<const Person*> return_vec;
    for (const auto& map_element_pair : person_map_) {
      return_vec.push_back(&(map_element_pair.second) );
    }
    return return_vec;
  }

  PersonList(){
    /* The following code is disabled because currently the std::map is used
    but it can be enabled again if the unordered map is desired.
    //Speed is more important than size
    person_map_.max_load_factor(0.5); 

    //By default we will have room for 4e5 persons.
    person_map_.reserve(400000);
    */
  }
  
  PersonList(const PersonList&) = delete; // Avoid accidental passing by value. 
  PersonList& operator=(const PersonList&) = delete; 


private:
  std::map <int, Person> person_map_;
};

} // !namespace soa1
} // !namespace sv
#endif // !SOA1_CF_SV_PERSON_LIST_H
