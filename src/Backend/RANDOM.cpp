#include "Backend/RANDOM.hpp"

#include <random>

RANDOM::RANDOM( discrete_t associativity , [[maybe_unused]] discrete_t nstes ) : SubstitutionPolitics( associativity) {
}

RANDOM::~RANDOM() {       
}

discrete_t RANDOM::GetBlock( [[maybe_unused]] discrete_t index ) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist( 0 , this->associativity - 1 );
    return dist(gen);

} 

void RANDOM::Refresh( [[maybe_unused]] discrete_t index, [[maybe_unused]] discrete_t block) {
    return;
}


