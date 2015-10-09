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
GOAL: Provide a distribution for the relationship duration

INPUT: A quantile (value between 0 and 1). 0 will result in a very short
relation time 1 will result in a very long relation time.

IMPLEMENTATION: Currently we use a gamma distribution for which the mean and
variance can be set in the parameters_pack.

OUPUT: A relationship duration.
*/

#ifndef ALJE_RG_DUR_DISTRIBUTION_H
#define ALJE_RG_DUR_DISTRIBUTION_H

#include <string>
#include <cassert>
#include <boost/math/distributions/gamma.hpp>

namespace soa1 { // soa is the dutch equivalent of sti
namespace rg {   // rg -> relationship generation
namespace dur {  // dur-> duration

class Distribution {
public:

  double DurationGivenPercentile(double percentile) {
    return boost::math::quantile(gamma_distribution_, percentile);
  }

  // In order to be able to easily change the underlying distribution we 
  // construct the distribution using mean and variance
  Distribution(std::string type, double mean, double variance) : 
        gamma_distribution_(mean * mean / variance, variance / mean) {
    // This an additional check to ensure that people watching only the
    // parameterpack know the type of distribution
    assert(type == "gamma" &&  "Error in soa1::rg::dur::distribution "
      "the distribution process uses a gamma distribution but in the paramters"
      " pack another distribution has been specified.");
    assert(mean * mean >= variance && "Error in soa1::rg::dur::distribution "
      "the distribution tried to initialize with a variance smaller than "
      "mean^2 which cannot occur for a positive function. Sorry :-)");
  }

  Distribution() = delete; // We need to know which gamma distribution
  // Copyable & assignable: yes

private:
  boost::math::gamma_distribution<> gamma_distribution_;

};//!class Distribution
}// !namespace dur
}// !namespace rg
}// !namespace soa1

#endif //!ALJE_RG_DUR_DISTRIBUTION_H