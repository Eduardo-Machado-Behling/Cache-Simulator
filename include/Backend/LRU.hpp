#ifndef LRU_HPP
#define LRU_HPP

#include "Backend/SubstitutionPolitics.hpp"

#include <vector>
#include <queue>

class LRU : public SubstitutionPolitics
{
private:
    std::vector< std::queue< discrete_t > > priority;
    std::queue< discrete_t > bucket; 
public:
    LRU( discrete_t associativity , discrete_t nstes ); 
    ~LRU();
    discrete_t GetBlock( discrete_t index ) override;
    void Refresh(discrete_t index, discrete_t block) override;
};

#endif