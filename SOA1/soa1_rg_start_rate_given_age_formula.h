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
GOAL: A class which holds the function which gives the rate of acquiring new
relationships as a function of the days a person is alive.

INPUT: Days alive

IMPLEMENTATION: Fitting a cubic curve (ax^3+bx^2+cx+d) to the values supplied in 
the parameter pack. The values of a,b,c, and d are precalculated in the
constructor. This requires a lot of math and will be supplied in the support
documentation of this file

OUTPUT: Depending on the function called, the rate of acquiring new partners or
the derivative (with respect to t) of the rate of acquiring new partners.
*/

#ifndef SOA1_RG_RATE_GIVEN_AGE_FORMULA
#define SOA1_RG_RATE_GIVEN_AGE_FORMULA

#include "soa1_parameters_pack.h"

namespace soa1 {
namespace rg {    // rg -> relationship generation
namespace start { // when do relationships start?

class RateGivenAgeFormula {
public:

  double rate_primitive(double age_in_days){
    // See top for more info.
    double x = age_in_days; // For easier readibility below.
    return a_/4.0*x*x*x*x + b_/3.0*x*x*x + c_/2.0*x*x + d_*x;
  }

  double rate_function(double age_in_days){
    // See top for more info.
    double x = age_in_days; // For easier readibility below.
    return a_*x*x*x + b_*x*x + c_*x + d_;
  }

  double rate_derivative(double age_in_days){
    // See top for more info.
    double x = age_in_days; // For easier readibility below.
    return 3.0*a_*x*x + 2.0*b_*x+c_;
  }

  RateGivenAgeFormula(const parameters::RelationGenerationStart
    rg_parameters) {
    // The constructor fits a cubic curve to the parameters supplied in 
    // the parameter pack. The derivation of these equations will be supplied
    // in the support wiki.

    // First we rename some variables to make the calculus more readible below.
    // Also in order to be comprehensive the parameter pack are values per year
    // in order to fit with the inner workings of the model (timestep = 1 day)
    // we multiply by 365 to convert from years to days.
       
    double b = rg_parameters.sexual_onset * 365; // b for begin 
    double e = rg_parameters.sexual_stop * 365;  // e for end
    double s = rg_parameters.rate_given_age_formula_skew; // s for skew.
    // A substitution to make life easier a couple of lines down.
    double p = 1.0 / (e-b); 

    // We start by calculating a,b,c,d to get the right shape 
    // (not the correct height yet). We rescale the polynomial to have
    // an average rate of 1 (hence the *12 at the end)
    double a_avg_rate_equal_to_one = p*p*p*(2*s - 1) * 12;
    double b_avg_rate_equal_to_one =
           p*p*(3*b*p - 6*b*s*p + 1 - 3*s) * 12;
    double c_avg_rate_equal_to_one =
           p*(6*b*b*s*p*p - 3*b*b*p*p + 6*b*s*p - 2*b*p + s) * 12;
    double d_avg_rate_equal_to_one =
           (b*b*b*p*p*p - 2*b*b*b*s*p*p*p + b*b*p*p - 3*b*b*p*p*s - b*s*p) *12;

    // Now that we have a polynomial with the right width we will combine this
    // fitted polynomial with a constant in order to later be able to test the
    // effect of age_related effect (does a constant really produce different
    // results when compared to a constant line?). The combination is done by a
    // weighted average of the shape found above (average rate 1) and a 
    // constant with the value of 1 (also average rate 1) using the weight w
    // where 1=fully the shape found above, 0 = a constant rate.
    double w = rg_parameters.age_effect_strength; 
    double a_including_strength = w * a_avg_rate_equal_to_one + 0.0;
    double b_including_strength = w * b_avg_rate_equal_to_one + 0.0;
    double c_including_strength = w * c_avg_rate_equal_to_one + 0.0;
    double d_including_strength = w * d_avg_rate_equal_to_one + (1.0-w);

    // Now we have a function with the right width and shape. Now we still
    // have to adjust it to the right height.
    double current_total_partners = e-b; 
    double desired_total_partners = 
          rg_parameters.average_total_lifetime_n_partners;
    a_ = a_including_strength / current_total_partners * desired_total_partners;
    b_ = b_including_strength /current_total_partners * desired_total_partners;
    c_ = c_including_strength /current_total_partners * desired_total_partners;
    d_ = d_including_strength /current_total_partners * desired_total_partners;
  }

private:
  // Coefficients of the cubic function which will be used to approximate
  // the rate given age formula.
  double a_,b_,c_,d_; 
  RateGivenAgeFormula() = delete; // We need parameters

};

} // !namespace start
} // !namespace rg
} // !namespace soa1

#endif // !SOA1_RG_RATE_GIVEN_AGE_FORMULA