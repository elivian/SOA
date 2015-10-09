
/*
#include <iostream>
#include "soa1_parameters_pack.h"
#include "soa1_rg_start_next_relation_time.h"
#include "soa1_rg_start_rate_given_age_formula.h"
#include "alje_rng_seed_generator.h"
*/

/*
//TEMP FOR DEBUG
#include "soa1_sv_person.h"
#include "soa1_sv_relation_list.h"
#include "soa1_sv_relation.h"
*/

#include "soa1_simulation.h"

#include "alje_process_x.h"
#include "alje_process_x_generator.h"


int main(){

  soa1::Simulation simulation;

  simulation.Start();
  
  /*
  
  const double average,
                               const double weight_average,
                               const double weight_short_history,
                               const double weight_long_history,
                               const double short_decay_rate,
                               const double long_decay_rate,
                               const std::uint32_t seed)
  
  */

  /*
  soa1::sv::State state_temp;
  alje::EventManager evm;
  std::unique_ptr<alje::Event> end_event = std::make_unique<soa1::rg::ev::EndRelation>(state_temp, 1, 30);
  evm.Add(std::move(end_event));

  evm.NotifyChannel2(1,0);
  */


  /*
  soa1::cf::sv::Person person1;
  soa1::cf::sv::Person person2;
  soa1::cf::sv::Person person3;
  soa1::cf::sv::Person person4;
  soa1::cf::sv::Person person5;
  soa1::cf::sv::Person person6;
  


  soa1::sv::Relation relation0(5, 7, 44);
  soa1::sv::Relation relation1(5, 8, 44);
  soa1::sv::Relation relation2(5, 9, 44);
  soa1::sv::Relation relation3(5, 10, 49);
  soa1::sv::Relation relation4(5, 11, 12);
  soa1::sv::Relation relation5(7, 5, 101);


  soa1::sv::RelationList RL;
  RL.Insert(relation0);
  RL.Insert(relation1);
  RL.Insert(relation2);
  RL.Insert(relation3);
  RL.Insert(relation4);
  RL.Insert(relation5);

  RL.RemoveGivenRelationID(4);
  */

  /*
//  soa1::sv::Relation* tempasd2 = RL.PointerGivenRelationID(5);
  //tempasd2->time_end();
  int n = 10;
  int q = 312;
//  auto blaat = tempasd2;

  auto temp = RL.FindRelationIDsGivenPersonID(5);

  //soa1::sv::Relation* temp8923 = RL.PointerGivenRelationID(8);
  
  for (auto& temp2 : temp){
    std::cout << temp2 << std::endl;
    const soa1::sv::Relation* temp_ptr = RL.PointerGivenRelationID(temp2);
   // int temp_tes = temp_ptr->time_start();
    int tempasda34 = temp_ptr->time_start();
    std::cout << RL.PointerGivenRelationID(temp2)->time_start() << std::endl;
  }


 // auto temp = person1;

 */

  /*
  alje::RngSeedGenerator seed_generator;
  const soa1::parameters::ParameterPack parameter_pack;
  soa1::rg::start::Main a_person(parameter_pack.relation_generation_start, seed_generator);


  for (int i = 0; i < 1000; ++i){
    if (i%10 == 0)
     std::cout << a_person.NextRelationTime() << std::endl;
  }
  
  */

  /*
  soa1::rg::start::RateGivenAgeFormula formula_temp(parameter_pack.relation_generation_start);
  double r = formula_temp.rate_function(29200);
  */
  
  return 0;
}