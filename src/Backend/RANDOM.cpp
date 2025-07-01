#include "Backend/SubstitutionPolitics.hpp"

#include <stdlib.h> 
#include <time.h>

class RANDOM : public SubstitutionPolitics
{
public:
    RANDOM( int associativity , int nstes ); 
    ~RANDOM();
    int GetBlock( int index ) override;
    void Refresh(int index, int block) override;
};

RANDOM::RANDOM( int associativity , int nstes ) : SubstitutionPolitics( associativity , nstes ) {
    srand (time(NULL));
}

RANDOM::~RANDOM() {       
}

int RANDOM::GetBlock( int index ) {
   return rand() % this->associativity;
} 
void RANDOM::Refresh(int index, int block) {
    return;
}


