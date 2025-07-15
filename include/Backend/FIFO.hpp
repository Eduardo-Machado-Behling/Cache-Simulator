#ifndef FIFO_HPP
#define FIFO_HPP

#include "Backend/SubstitutionPolitics.hpp"

#include <vector>
#include <queue>

class FIFO : public SubstitutionPolitics
{
private:
    std::vector< std::queue< discrete_t > > priority ;
public:
    FIFO( discrete_t associativity , discrete_t nstes ); 
    ~FIFO();
    discrete_t GetBlock( discrete_t index ) override;
    void Refresh(discrete_t index, discrete_t block) override;
};

#endif