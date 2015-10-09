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
GOAL: The parameter pack is THE location for all the parameters which determine
the inner workings of the model and might need to be changed.

INPUT: N/A

IMPLEMENTATION: There is one overall pack soa1::parameters::pack which
for every namespace (and subnamespace) contains a struct belonging to that
namespace.

OUTPUT: N/A

IMPORTANT NOTE: By default everything in the PARAMETER PACK is in YEARS, by
default everything in the MODEL is in DAYS. (reason: the model has timesteps
of days for accuracy, the paramter pack is in years for easy readibility)
*/

#ifndef SOA1_PARAMETERS_PACK_H
#define SOA1_PARAMETERS_PACK_H

#include <string>

namespace soa1 {
namespace parameters {

struct Demographics{
  int initial_population = 23800; // XXX 238.000 Sexually active population
};

struct RelationGenerationStart{

  // RG Start Stationary process
  double stat_process_average = 16;
  int stat_process_n_prearrivals = 1000000; // XXX #arrivals per person for initialization
  double weight_average = 0.5;          
  double weight_short_history = 0.5; 
  double weight_long_history = 0; 
  double short_decay_rate_days = 1.0/30.0 ;     // Unit is DAYS^-1 
  double long_decay_rate_days = 1.0/3650.0 ;   // Unit is DAYS^-1

  // These numbers are fit to the schorer/acs data.
  double sexual_onset = 15; // In years since birth
  double sexual_stop = 80;  // In years since birth
  double average_total_lifetime_n_partners = 300; //Approximated from the XXX return to 780 XXXX300
  // The model will fit a quadratic curve based on the parameters above.
  // But in addition one can supply a "skew"  (if the most active period of
  // people is not halfway between sexual onset and stop).
  // Skew = 0.5 -> No skew
  // Skew = 0   -> t(Peak rate) is about 1/3 of sexual lifetime (0 is minimum!)
  // Skew = 1   -> t(Peak rate) is about 2/3 of sexual lifetime (1 is maximum!)
  // See the soa1_rg_rate_given_age_formula in the support wiki for more info.
  // The data does not seem to indicate a skew.
  double rate_given_age_formula_skew = 0.5;
  // The data as described above is fitted with the data. In order to test if
  // age related effects really are important we include a strength parameter
  // if this parameter = 1 the model will completely fit to above parameters.
  // if this parameter = 0 all age-related effects will be ignored 
  // in this case a constant rate is assumed
  double age_effect_strength = 1; // xxx turn to 1
};

struct RelationDuration {
  std::string distribution = "gamma"; 
  double mean = 20; // in DAYS
  double variance = 40; // in DAYS^2 
  double monogamy = 0.5; //1 equals fully monogamous, 0 equals fully concurrent
};

struct MatchMaking {
  std::vector<std::vector<double>> age_groups{
    {15,20},
    {20,25},
    {25,30},
    {30,35},
    {35,40},
    {40,45},
    {45,50},
    {50,55},
    {55,60},
    {60,65},
    {65,70},
    {70,75},
    {75,80}  
  };
  std::string age_group_preference_distribution = "normal";
  double age_group_preference_sd = 12; 

  std::vector<std::vector<int>> duration_groups{
    {0,0},
    {1,1},
    {2,3},
    {4,7},
    {8,15},
    {16,31},
    {32,61},
    {62, 183},
    {184, 365},
    {366, 730},
    {731, 1825}, //2-5year
    {1826, 3560},
    {3561, std::numeric_limits<int>::max()} // the rest
  };
  // Matches only apply if duration is in the same groups.
  std::string duration_group_preference_distribution = "exact";

  // See the mm_parner_choice_matrix class for more details.
  double weight_new_database_update = 0.001;      // lower = better & slower 
  int n_relation_matrix_iterations = 50;          // higher = better & slower
  double group_estimate_error_tolerance = 0.001;  // lower is better & slower

};

struct TransmissionSexualBehavior {
  double sex_frequency = 1 / 3.0; // In times per day
  double condom_use = 0.6;        // percentage (1 being all the time).
};

struct TransmissionHIV {
  double base_rate = 0.005;        // Prob per unprotected anal intercourse.

  // How much more infective are you during certain periods in time?
  // EXAMPLE
  //  {0,1.5}, // From time 0 to time 200 a person is 1.5 times as infective.
  //  {200, 0.9},// From time 200-3650 a person is 0.9 times as effective.
  //  {3650,1.1} // For times over 3650 a person is 1.1 time as effective.
  std::vector<std::vector<double>> infectivity_over_time = {
    {0,5},
    {400, 0.9},
    {3650,1.1}
  }; // In days.
  //If a person has gonorrhea, how much more infective is he?
  double hiv_pos_has_gonorrhea_multiplier = 1.5; 
  double hiv_neg_has_gonorrhea_multiplier = 1.5;
};

struct TransmissionGonorrhea {
  double base_rate = 0.30;
  double probability_symptomatic = 0.6;
  int natural_cure_symptomatic = 45;
  int natural_cure_asymptomatic = 200; 
};

struct ParameterPack{
  Demographics demographics;
  RelationGenerationStart relation_generation_start;
  RelationDuration relation_generation_duration;
  MatchMaking matchmaking;
  TransmissionSexualBehavior sexual_behavior;
  TransmissionHIV hiv;
  TransmissionGonorrhea gonorrhea;
};


} // End of namespace parameters
} // End of namespace soa1

#endif
