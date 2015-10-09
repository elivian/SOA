/*
GOAL: This file keeps track of the relations persons are involved in.

IMPLEMENTATION: It used boost multiindex to be able to acces relations by the
ID of the persons involved in the relation.
*/

#ifndef SOA1_SV_RELATION_LIST_H
#define SOA1_SV_RELATION_LIST_H

#include <vector>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include "soa1_sv_relation.h"

namespace soa1{
namespace sv {  // sv -> status variables

struct RelationID{}; // For tagging in the boost multiindex.
struct Person1ID{};
struct Person2ID{};

typedef boost::multi_index_container<
  Relation,
  boost::multi_index::indexed_by<
    // Index by the id of the relation.
    boost::multi_index::ordered_unique<
      boost::multi_index::tag<RelationID>,
      BOOST_MULTI_INDEX_CONST_MEM_FUN(
        Relation, const int, relation_id
      )
    >,
    // index by the id of the first person
    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Person1ID>,
      BOOST_MULTI_INDEX_CONST_MEM_FUN(
        Relation, const int, person1_id
      )
    >,
    // Index by the id of the second person
    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Person2ID>,
      BOOST_MULTI_INDEX_CONST_MEM_FUN(
        Relation, const int, person2_id
      )
    >
  >
> RelationMultiIndex;

class RelationList{
public:
  void Insert(Relation relation_to_insert){
    relation_multi_index_.insert(relation_to_insert);
  }

  // Returns a vector of references to Relations
  std::vector<Relation&> FindRelationsGivenPersonID(int person_to_find){

    std::vector<Relation&> matching_id_list;
                                           
    auto it_person1 =                                                
      relation_multi_index_.get<Person1ID>().find(person_to_find);    
    while (it_person1 !=                                             
          relation_multi_index_.get<Person1ID>().end()){             
      if (it_person1->person1_id() != person_to_find) {
        // We iterate over the sorted relation_multi_index but this iterator 
        // doesn't know when to stop. So we need to manually stop when we are
        // no longer returning relations from this person.
        break;
      }
      const Relation* relation_to_add = *it_person1;
      matching_id_list.push_back(it_person1->relation_id());
      ++it_person1;
    }

    // For explanation see everything above (done for another person)
    auto it_person2 =
      relation_multi_index_.get<Person2ID>().find(person_to_find);
    while (it_person2 !=
      relation_multi_index_.get<Person2ID>().end()){
      if (it_person2->person2_id() != person_to_find)
        break;
      matching_id_list.push_back(it_person2->relation_id());
      ++it_person2;
    }
  
    return matching_id_list;
  }

  // Be careful with the pointer :)
  const Relation* PointerGivenRelationID(int id){

    auto it = relation_multi_index_.get<RelationID>().find(id);

    if (it == relation_multi_index_.get<RelationID>().end()){
      return NULL; // We didn't find anything!
    }

    //Dereference first (from iterator to Relation)
    const Relation& it_deref = *it;
    //and then get the address to return a pointer (from Relation to Relation*)
    return &it_deref;
  }

  void RemoveGivenRelationID(int id){
    auto iterator_to_erase = relation_multi_index_.get<RelationID>().find(id);
    // ID's are unique so we can just delete this iterator.
    relation_multi_index_.get<RelationID>().erase(iterator_to_erase);
  }
  
private:
  RelationMultiIndex relation_multi_index_;
};

} // !namespace sv 
} // !namespace soa1
#endif //  !SOA1_CF_RELATION_LIST_H