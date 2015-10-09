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
GOAL: Turning a list of values into the an estimated percentile. So 0.3 would
mean that an estimated 30% of the all values is lower than the value it is 
currently called with (and 70% higher). This class is 'online' meaning that you
can call it repeatedly with a single value and it will estimate a percentile 
based on all values passed so far. (so the first few calls will be really
inaccurate).

INPUT: Any integer values

IMPLEMENTATION: Keeps a sorted vector of all values this class has been called
with and determine where the new value fits in. I limit the vector to a maximum
size of X (see code for the value of X) in order to avoid unnecessary sorting
and unnecessary use of resources. This means that after X calls the database 
will no longer update.

OUTPUT: a lower and an upper percentile. This is done to allow for duplicates.
E.g. (0,0,0,0,1) what is the percentile of a 0 here? We return a lower and
upper bound and leave this up to the calling function to decide. 

NOTE: This class is part of the SOA1 namespace but with a little effort it
could be made into a more general class. Hence the general names. Extensions
can include:
i. allow any input type (templating)
ii. allow more customizability of the underlying database.
iii. Now the underlying database is sorted at every addition (=slow)

*/

#ifndef VALUES_TO_PERCENTILES_H
#define VALUES_TO_PERCENTILES_H

#include <vector>     // For the database
#include <utility>    // For pair (lower percentile/upper percentile)
#include <iterator>   // For std::distance to convert iterator to key.
#include <algorithm>  // For sorting and finding

namespace soa1{       // soa is the dutch word for STI
namespace rg {        // rg -> relationship generation
namespace dur {       // dur -> duration

class ValuesToPercentiles {
public:
  std::pair<double, double> GetLowerUpper(int value) {

    // 1. Find the percentile
    // 2. Possibly add to the database
    // 3. Return the results

    // 1. Find the percentile
    // ----------------------
    // If there are 8 values in the database, there are 9 possible positions
    // for the new value (7 between the values in the database and 2 at the 
    // ends)
    double possible_positions_to_insert =
      static_cast<double>(sample_database_.size() + 1);

    // Now find the first and last position the supplied value would fit while
    // still keeping everything sorted.
    // lower_bound returns an iterator pointing to the first element in the 
    //    range[first, last) that is not less than (i.e.greater or equal to) 
    //    value.
    // upper_bound returns an iterator pointing to the first element in the 
    //    range[first, last) that is greater than  
    //    value.
    auto lower_it = std::lower_bound(sample_database_.begin(),
      sample_database_.end(), value);
    auto upper_it = std::upper_bound(sample_database_.begin(),
      sample_database_.end(), value);
    // These are iterators to the elements before which we would want to insert
    // so these iterators (a bit coincidentally) point to the position where we
    // would want to insert. So lower_position can be 0 if it would be the
    // lowest value of all. So all we have to do is convert these iterators to
    // the key, we use the distance function for this.
    int lower_position = std::distance(sample_database_.begin(), lower_it);
    int upper_position = std::distance(sample_database_.begin(), upper_it);

    // Now, if there are 9 possible positions to insert the lower_position and
    // upper_position have values between 0 and 8. Now we don't want 0 to 
    // return a 0 percentile (reason: this is not a conservative estimate). So
    // we add 0.5 such that the lower_position can have values between 0.5 and 
    // 8.5.
    double percentile_lower =
      (lower_position + 0.5) / possible_positions_to_insert;
    double percentile_upper =
      (upper_position + 0.5) / possible_positions_to_insert;

    // 2. If needed add this value to the database
    // -------------------------------------------
    // It is easy to insert the new value in the right place at the vector. We
    // already know from part 1 that upper_it is the first element which is 
    // greater than the supplied_value so we insert just before this (using 
    // insert). I could have used lower_it, but this would insert at an earlier
    // position in the vector which would therefore require more moving of
    // values.
    if (sample_database_.size() < 100000)
      sample_database_.insert(upper_it, value);

    // 3. return the results.
    // ----------------------
    return std::make_pair(percentile_lower, percentile_upper);
  }

private:
  std::vector<int> sample_database_; // The sample database.

};//!class ValuesToPercentiles
}// !namespace dur
}// !namespace rg
}// !namespace soa1
#endif //!VALUES_TO_PERCENTILES_H