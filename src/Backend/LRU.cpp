#include "Backend/SubstitutionPolitics.hpp"

#include <vector>
#include <queue>

class LRU : public SubstitutionPolitics
{
private:
    std::vector< std::queue< int > > priority;
    std::queue< int > bucket; 
public:
    LRU( int associativity , int nstes ); 
    ~LRU();
    int GetBlock( int index ) override;
    void Refresh(int index, int block) override;
};

LRU::LRU( int associativity , int nstes ) : SubstitutionPolitics( associativity , nstes ) {
    priority = std::vector< std::queue< int > >( nstes );
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

int LRU::GetBlock( int index ) {
    int temp = priority[index].front();
    priority[index].pop();
    priority[index].push( temp );
    return temp;
}
void LRU::Refresh(int index, int block) {
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


