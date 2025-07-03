#ifndef LRU_HPP
#define LRU_HPP

#include "Backend/SubstitutionPolitics.hpp"

#include <vector>
#include <list>

class LRU : public SubstitutionPolitics
{
protected:
    std::vector< std::list< discrete_t > > priority;
public:
    LRU( discrete_t associativity , discrete_t nstes ); 
    ~LRU();
    discrete_t GetBlock( discrete_t index ) override;
    void Refresh(discrete_t index, discrete_t block) override;
};

#endif