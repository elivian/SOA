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
GOAL: Given the soa1_parameter_pack it will generate times at which a relation 
starts for an individual (in PersonTime, so days since birth). So every 
individual needs their own instance of this RelationStart class.

INPUT: 
On construction: rg_parameters (relationship generation parameters)
On callingNextRelationTime: the current age  of the person

IMPLEMENTATION: A wrapper around alje::stationary_auto_induced_process which
will tailor the stationary.a.i.process to the fit the parameters in the
parameter pack. The main challenge is to transform the time of the 
Stationary.a.i.pack to fit a nonstationary persons life (who has more relations
halfway through their sexual carreer.

OUTPUT: The age (in days since birth) the person starts his next relation.

MORE INFORMATION: look at the support wiki (probably www.elivian.nl) 

*/
#ifndef SOA1_RG_START_NEXT_RELATION_PERSON_TIME_H
#define SOA1_RG_START_NEXT_RELATION_PERSON_TIME_H

#include <limits>
#include "soa1_parameters_pack.h"
#include "soa1_rg_start_rate_given_age_formula.h"
#include "alje_process_x.h"
#include "alje_rng_seed_generator.h"

namespace soa1 {    // soa is the dutch word for sti
namespace rg {      // relation generation
namespace start {   // at what points in time does a new relationship start?

class NextRelationPersonTime{

 public:
  int Get(){
    /*
    GOAL: The public function which can be called to get the next moment in time
    that the person owning this class gets a relation.

    IMPLEMENTATION: A wrapper around alje::stationary_auto_induced_process for
    which the time is then stretched/compressed to fit different rates at 
    different ages. This is done according to the functions specified in the
    parameter pack. In order to do this stretching/compressing we need to solve
    an equation where at this point in time we use a simple Newton Raphson
    estimate.

    OUTPUT: (int) the next time in days since birth. 
    These will be non-decreasing.
    */
    
    // Get the time for the stationary process. 
    // XXX say something about the average of the stationary distribution
    // where does this happen?

    double next_arrival = stationary_process_.NextArrival();
	  assert(next_arrival >= 0 && "error in soa1_rg_start_next_relation_time"
		" a next arrival with negative value was passed.");

    stationary_process_total_time += next_arrival;
    
    // The real process is not stationary so convert this to a real time in
    // days since birth. 
    // XXX  real_person_times returns -infinity?
    double real_person_time = 
      ConvertStationaryTimeToRealPersonTime(stationary_process_total_time);
    
    // If no more relations will occur, return "infinity"
    if (real_person_time == -1) return std::numeric_limits<int>::max();

    // We want only discrete number of days so return an integer
    // static_casts rounds down (for positive numbers).
    return static_cast<int>(real_person_time); 
  }
 
  // The Constructor sets up the stationary process. Note that since the
  // parameter pack is in years (and the model is in days) we need to 
  // convert the relevant variables to days.
  NextRelationPersonTime(
       const parameters::RelationGenerationStart rg_start_parameters, 
       alje::RngSeedGenerator& rng_seed) : 
                  rate_formula_(rg_start_parameters),
                  sexual_onset_(rg_start_parameters.sexual_onset * 365),
                  sexual_stop_(rg_start_parameters.sexual_stop * 365),
                  stationary_process_(
                    rg_start_parameters.stat_process_average / 365.0,
                    rg_start_parameters.weight_average,
                    rg_start_parameters.weight_short_history,
                    rg_start_parameters.weight_long_history,
                    rg_start_parameters.short_decay_rate_days,
                    rg_start_parameters.long_decay_rate_days,
                    rng_seed.Get()) {}

  // The Constructor sets up the stationary process. Note that since the
  // parameter pack is in years (and the model is in days) we need to 
  // convert the relevant variables to days.
  NextRelationPersonTime(
    const parameters::RelationGenerationStart rg_start_parameters,
    alje::RngSeedGenerator& rng_seed, 
    alje::ProcessXGenerator& process_x_generator) :
    rate_formula_(rg_start_parameters),
    sexual_onset_(rg_start_parameters.sexual_onset * 365),
    sexual_stop_(rg_start_parameters.sexual_stop * 365),
    stationary_process_(process_x_generator.Get()) {
  }

  NextRelationPersonTime() = delete; // We need parameters!
  NextRelationPersonTime(const NextRelationPersonTime& n) = default;
  NextRelationPersonTime& operator=(const NextRelationPersonTime& n) = delete;

 private:
  double stationary_process_total_time = 0;
  double sexual_onset_; // By default in the model everything is in days.
  double sexual_stop_;  // By default in the model everything is in days.

  //The underlying process
  alje::ProcessX stationary_process_; 
  rg::start::RateGivenAgeFormula rate_formula_;

  inline double f(const double real_time_guess, const double required_stat_time){
    /* 
    GOAL: Make sure the ConvertStationaryTimeToRealPersonTime function only has
    to worry about solving f(x,constant) = 0 
    (with the helper f_deriv provided below).

    INPUT: real_time (can be thought of as the x) and the 
       stationary time (can be thought of as the y, what you want to solve for)

    IMPLEMENTATION: 
    int_{sexual_onset}^{real_person_time_guess}
                  rate_given_age_formula = unit_stat_process
    so 
      int_{0}^{real_person_time_guess}rate_given_age_formula - 
        int{0}^{real_person_time_guess}rate_given_age_formula=unit_stat_process
    so the formula which should = 0 is
      rate_formula.rate_primitive(real_time_guess) -
        rate_formula.rate_primitive(sexual_onset_) - unit_stat_process = 0
    so converting the stat process to a unit stat process we find the formula 
    used here
    OUTPUT: real_time_guess converted to unit stat time - 
      required stationary time converted to unit stat time.
    Or more easy: a function which will be closer to 0 as the real_time_guess
    given the required_stat_time be closer to eachother.
       
    */

    // For debugging we use intermediate values (a,b,c,d)
    auto a = rate_formula_.rate_primitive(real_time_guess);
    // Stationary time starts running at sexual onset
    auto b = rate_formula_.rate_primitive(sexual_onset_); 
    // We convert the stationary_process to "unit" speed. This allows us to use
    // the rate_given_age_formula (that formula assumes unit rate)
    auto c = required_stat_time * stationary_process_.average_rate();
    auto d = a - b - c;
    return d;
  }

  inline double f_deriv(const double real_time){
    // The derivative of the function above, required for Newton Raphson method
    return rate_formula_.rate_function(real_time);
  }

  double ConvertStationaryTimeToRealPersonTime(const double stat_time){
   /* 
   GOAL: Provide a wrapper around alje::process_x for
   which the time is then stretched / compressed to fit different rates at
   different ages. This is done according to the functions specified in the
   parameter pack.In order to do this stretching / compressing we need to solve
   an equation where at this point in time we use a simple Newton Raphson
   estimate.

   IMPLEMENTATION: alje::process_x gives a stationary process with a mean rate
   according to soa1::paramterers_pack average. In order to facilitate the
   solving we defined the formulas f and f_deriv in such a way that this
   function only needs to worry about solving f = 0.

   Output: The time (in days since birth) of the next relation or -1 if no
   more relations will occur.
   */

    //Newton Raphson 

    // Every iteration the estimated solution gets more accurate. The 
    // last_correction variable keeps track of the difference between the
    // last estimation and the current estimation and is used to terminate
    // the algorithm if sufficient accuracy is reached.
    double last_correction = std::numeric_limits<double>::max();

    // A first estimation is exactly halfway in the sexual career.
    double current_estimate = 
          (sexual_stop_ - sexual_onset_)/2.0 + sexual_onset_;

    // We want it to be correct up until 1/10th of a day (just to be sure)
    // f is defined in this class (just above this function).
    while (std::abs(last_correction) > 0.1){
      double old_estimate = current_estimate;
      current_estimate = current_estimate - 1.0 * f(current_estimate, stat_time) /
        f_deriv(current_estimate);

      // Numerical stability check. To ensure stability of the netwon Raphson
      // method we do not allow estimates to exceed the bounds (or come near
      // them)
      if (current_estimate >= sexual_stop_ - 1)
        current_estimate = sexual_stop_ - 1; 
      if (current_estimate <= sexual_onset_ + 1)
        current_estimate = sexual_onset_ + 1;

      // We store the change in this iteration in order to determine if the
      // algorithm should be terminated.
      last_correction = current_estimate - old_estimate;
    }

    // If we hit the upper bound no more relations will occur before the end 
    // of the (sexual) life XXX change this into an "infinity"?
    if (current_estimate >= sexual_stop_ - 1)
      return -1; // Out of bounds! 

    return current_estimate;
}

}; // ! class RelationStart
} // ! namespace start
} // ! namespace rg
} // ! namespace soa1



#endif