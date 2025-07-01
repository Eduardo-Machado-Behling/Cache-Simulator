#ifndef SUBSTITUtION_POLITICS_HPP
#define SUBSTITUtION_POLITICS_HPP

#include "common/CacheSpecs.hpp"

class SubstitutionPolitics
{
protected:
    int associativity , nstes;
    SubstitutionPolitics( int associativity , int nsets );
public:
    virtual ~SubstitutionPolitics() = default;
    virtual int GetBlock( int index ) = 0;
    virtual void Refresh( int index , int block ) = 0;
};

SubstitutionPolitics::SubstitutionPolitics( int associativity , int nsets ) {
    this->associativity = associativity;
    this->nstes = nstes;
}

#endif