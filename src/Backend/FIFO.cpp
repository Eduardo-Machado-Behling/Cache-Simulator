#include "Backend/FIFO.hpp"

FIFO::FIFO( discrete_t associativity , discrete_t nstes ) : SubstitutionPolitics( associativity ) {
    priority = std::vector< std::queue< discrete_t > >( nstes );
    for ( size_t index = 0 ; index < nstes ; index++ ) {
        for( size_t block = 0 ; block < associativity ; block++ ) {
            priority[index].push( block );
        }
    }
}

FIFO::~FIFO() {
    priority.clear();
}

discrete_t FIFO::GetBlock( discrete_t index ) {
    discrete_t temp = priority[index].front();
    priority[index].pop();
    priority[index].push( temp );
    return temp;
}
void FIFO::Refresh( [[maybe_unused]]  discrete_t index, [[maybe_unused]]discrete_t block) {
    return;
}


