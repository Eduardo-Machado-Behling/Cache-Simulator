#include "Backend/SubstitutionPolitics.hpp"

#include <vector>
#include <queue>

class FIFO : public SubstitutionPolitics
{
private:
    std::vector< std::queue< int > > priority ;
public:
    FIFO( int associativity , int nstes ); 
    ~FIFO();
    int GetBlock( int index ) override;
    void Refresh(int index, int block) override;
};

FIFO::FIFO( int associativity , int nstes ) : SubstitutionPolitics( associativity , nstes ) {
    priority = std::vector< std::queue< int > >( nstes );
    for ( size_t index = 0 ; index < nstes ; index++ ) {
        for( size_t block = 0 ; block < associativity ; block++ ) {
            priority[index].push( block );
        }
    }
}

FIFO::~FIFO() {
    priority.clear();
}

int FIFO::GetBlock( int index ) {
    int temp = priority[index].front();
    priority[index].pop();
    priority[index].push( temp );
    return temp;
}
void FIFO::Refresh(int index, int block) {
    return;
}


