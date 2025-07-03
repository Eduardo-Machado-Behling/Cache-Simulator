#ifndef SUBSTITUTION_POLITICS_HPP
#define SUBSTITUTION_POLITICS_HPP

#include "common/CacheSpecs.hpp"

class SubstitutionPolitics
{
protected:
    discrete_t associativity;
    public:
    SubstitutionPolitics( discrete_t associativity ){
        this->associativity = associativity;
    }
    virtual ~SubstitutionPolitics() = default;
    virtual discrete_t GetBlock( discrete_t index ) = 0;
    virtual void Refresh( discrete_t index , discrete_t block ) = 0;
};

#endif