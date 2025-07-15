#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "Backend/SubstitutionPolitics.hpp"

#include <stdlib.h> 
#include <time.h>

class RANDOM : public SubstitutionPolitics
{
public:
    RANDOM( discrete_t associativity , [[maybe_unused]] discrete_t nstes ); 
    ~RANDOM();
    discrete_t GetBlock( [[maybe_unused]] discrete_t index ) override;
    void Refresh( [[maybe_unused]] discrete_t index, [[maybe_unused]] discrete_t block) override;
};

#endif