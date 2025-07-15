#include "Backend/LRU.hpp"

LRU::LRU( discrete_t associativity , discrete_t nstes ) : SubstitutionPolitics( associativity ) {
    priority = std::vector< std::list< discrete_t > >( nstes );
    for ( size_t index = 0 ; index < nstes ; index++ ) {
        for ( size_t block = 0 ; block < associativity ; block++ ) {
            priority[index].push_back( block );
        }
    }
}

LRU::~LRU() {       
    priority.clear();
}

discrete_t LRU::GetBlock( discrete_t index ) {
    discrete_t temp = priority[index].front();
    priority[index].pop_front();
    priority[index].push_back( temp );
    return temp;
}
void LRU::Refresh(discrete_t index, discrete_t block) { 
    priority[index].remove( block );
    priority[index].push_back( block );
}


