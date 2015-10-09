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
GOAL: Things can happen to a person. Some events might need to be updated. The
personID is then send to the alje_event_manager on channel 1. But in order to
identify WHAT happened to this person the event_manager allows us to send an
integer. But since integers aren't nice we use an old-fashioned enum in a 
namespace to determine what happened.
*/


#ifndef SOA1_SV_PERSON_EVENT_CODES_H
#define SOA1_SV_PERSON_EVENT_CODES_H

namespace soa1{
namespace sv{ // sv -> status variables
namespace person_event_codes{
enum PersonEventCodes{
  DEATH,
  GN_CURED // Cured of Gonorrhea
};

}// !namespace soa1
}// !namespace sv
}// !namespace person_event_codes



#endif //!SOA1_SV_PERSON_EVENT_CODES_H