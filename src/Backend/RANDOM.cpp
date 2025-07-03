#include "Backend/RANDOM.hpp"

RANDOM::RANDOM( discrete_t associativity , [[maybe_unused]] discrete_t nstes ) : SubstitutionPolitics( associativity) {
    srand ( ( unsigned int ) time(NULL));
}

RANDOM::~RANDOM() {       
}

discrete_t RANDOM::GetBlock( [[maybe_unused]] discrete_t index ) {
   return ( discrete_t ) rand() % this->associativity;
} 

void RANDOM::Refresh( [[maybe_unused]] discrete_t index, [[maybe_unused]] discrete_t block) {
    return;
}


