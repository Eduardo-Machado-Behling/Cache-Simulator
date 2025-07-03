#include "Backend/LRU.hpp"

LRU::LRU( discrete_t associativity , discrete_t nstes ) : SubstitutionPolitics( associativity ) {
    priority = std::vector< std::queue< discrete_t > >( nstes );
    for ( size_t index = 0 ; index < nstes ; index++ ) {
        for ( size_t block = 0 ; block < associativity ; block++ ) {
            priority[index].push( block );
        }
    }
}

LRU::~LRU() {       
    priority.clear();
    bucket = {};
}

discrete_t LRU::GetBlock( discrete_t index ) {
    discrete_t temp = priority[index].front();
    priority[index].pop();
    priority[index].push( temp );
    return temp;
}
void LRU::Refresh(discrete_t index, discrete_t block) {
    while ( priority[index].empty() ) {
        if( priority[index].front() != block ) {
            bucket.push( priority[index].front() );
            priority[index].pop();
        } else {
            priority[index].pop();
        }
    }
    priority[index].swap( bucket );
    priority[index].push( block );
    bucket = {};
}


