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
GOAL: The main class which given the time until the next relation gives the
duration of the relation.

INPUT: Time until the next relation of this person.

IMPLEMENTATION: 
Step 1: Find out which percentile the interrelation belongs to 
        (0=shortest 1 =longest) using the values_to_percentiles class.
Step 2: Draw a random number between 0 and 1.
Step 3: Combine the values of step 1 and 2 using distribution free association.
        That class ensures a homogenous output between 0 and 1. As a weight for
        the values of step 1 and 2 vary between 
        * Fully based on interrelation time of step 1 in case monogamy = 1 
          (weight step 1 = 1, weight step 2 = 0)
        * Fully random in case monogamy = 0
          (weight step 1 = 0, weight step 2 = 1)

OUTPUT: The duration (in days) of this relation. The lowest value = 0. So if
you want relations to take at least 1 day I suggest you add 1.

NOTE: A thing which kind of breaks encapsulation is that this class needs the
distribution of the interrelationtimes. For more ease of use this class
approximates this distribution by the first X interrelation times. So if the 
interrelationtime distribution changes this will break this class.

NOTE2: This class might result in durations which are slightly lower on
average than one might expect based on the dur::distribution. This can be
caused by the conversion from a continuous distribution to discrete duration.
We need to round down in this conversion (otherwise the shortest duration would
have the frequency underestimated by a factor 2).
*/

#ifndef SOA1_RG_DUR_GET_DURATION_H
#define SOA1_RG_DUR_GET_DURATION_H

#include <utility> // For pair which is returned by values_to_percentiles. 
#include <random>  // For if we have duplicates in interrelation times

#include "alje_rng_seed_generator.h"
#include "alje_distribution_free_association.h"
#include "soa1_parameters_pack.h"
#include "soa1_rg_dur_values_to_percentiles.h"
#include "soa1_rg_dur_distribution.h"

namespace soa1 {  // soa is the dutch equivalent of STI
namespace rg {    // rg -> relationship generation
namespace dur {   // dur -> Duration

class GetDuration {
public:
  int Get(int interrelation_time) {

    // Find out which percentile this interrelation belongs to. This is 
    // slightly more advanced than it looks because the GetPercentile class
    // bases its information about the distribution on the previous 
    // inter_relation_times supplied. Furthermore it returns a range in 
    // which the percentile will lie (this is done to allow for duplicates,
    // in 0,0,0,0,1, what percentile does the 0 belong to?).
    std::pair<double, double> inter_relation_percentile_lower_upper =
      val_to_percentile_.GetLowerUpper(interrelation_time);
    double lower_percentile = inter_relation_percentile_lower_upper.first;
    double upper_percentile = inter_relation_percentile_lower_upper.second;
    // By default choose the lower_percentile.
    double inter_relation_percentile = lower_percentile;
    // But sometimes we really have a range of percentiles to choose from.
    if (upper_percentile - lower_percentile > 0.001) {
      // In that case choose one at random in this range.
      inter_relation_percentile =
        (upper_percentile - lower_percentile) * unit_real_distribution_(rng_)
        + lower_percentile;
    }
    // now lower_percentile contains the estimated percentile this
    // interrelation_time belongs to.

    // We want duration percentiles to be partly random and partly dependent on
    // the inter_relation_time. This is done by the 
    // alje::distribution_free association class. This class also ensures that
    // if the input is homogenous between 0 and 1 so will the output be.

    double duration_percentile = association_.CombineValues
      ({unit_real_distribution_(rng_), inter_relation_percentile});

    // Now this duration_percentile has an association with the
    // interrelation_time. Now we use a distribution to convert this percentile
    // to the value. 
    // We round down here which might result in an approximately 0.5 days
    // lower duration (but rounding down is required because otherwise the 
    // shortest duration would be underestimated by a factor 0.5)


    auto ret_value = static_cast<int>(
      duration_distribution_.DurationGivenPercentile(duration_percentile));
    return ret_value;
  }

  GetDuration(    // Input parameters
    soa1::parameters::RelationDuration dur_parameters, 
    alje::RngSeedGenerator& seed_gen
  ):
    association_( // Initializiation list
      {1.0 - dur_parameters.monogamy, dur_parameters.monogamy}
    ), // See top->implem
    rng_(seed_gen.Get()),
    unit_real_distribution_(0,1), 
    duration_distribution_(
      dur_parameters.distribution, 
      dur_parameters.mean,
      dur_parameters.variance
    )
  {               // Constructor body
      assert(dur_parameters.monogamy >= 0 && dur_parameters.monogamy <= 1 && 
        "Error in "
        "soa1::rg::dur::GetDuration->constructor. A monogamy parameter which "
        "is not between 0 and 1 has been supplied");
  }

  GetDuration() = delete; // (need monogamy parameter!)
  GetDuration(const GetDuration& s) = default; // inter_relation_to_percentile
  GetDuration& operator=(const GetDuration& s) = delete;  

private:
  soa1::rg::dur::ValuesToPercentiles val_to_percentile_;
  std::minstd_rand rng_;
  std::uniform_real_distribution<> unit_real_distribution_;
  alje::DistributionFreeAssociation association_;
  soa1::rg::dur::Distribution duration_distribution_;

};

} // !namespace dur
} // !namespace rg
} // !namespace soa1

#endif// !SOA1_RG_DUR_GET_DURATION_H