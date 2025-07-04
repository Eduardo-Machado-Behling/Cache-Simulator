#include "Backend/Cache.hpp"
#include "Backend/LRU.hpp"
#include "Backend/RANDOM.hpp"
#include "Backend/FIFO.hpp"

#include <iostream>
#include <random>

Cache::Cache( std::span< std::string > command) : Backend(setCacheSpecs(command)){
  cache = std::vector< std::vector< CacheBlock > >( __specs.nsets );
  for ( discrete_t index = 0 ; index < __specs.nsets ; index++ ) {
    cache[index] = std::vector< CacheBlock >( __specs.assoc );
  }
  switch ( __specs.substitutionPolitics ) {
  case REPL::FIFO:
    substitutionPolitics = std::make_unique<FIFO>( __specs.assoc , __specs.nsets );
    break;
  case REPL::RANDOM:
    substitutionPolitics = std::make_unique<RANDOM>( __specs.assoc , __specs.nsets );
    break;
  case REPL::LRU:
    substitutionPolitics = std::make_unique<LRU>( __specs.assoc , __specs.nsets );
    break;
  }
}
Cache::~Cache() {
  cache.clear();
}

auto Cache::process([[maybe_unused]] addr_t addr) -> CacheAccess & {
  __report.accesses++;
  bool isFullBlock , isFull;
  discrete_t index = 0;  
  if( __specs.nsets > 1 ) {
    index = ( addr >> ( __specs.bits.offset ) ) & ( ( 1 << __specs.bits.index ) - ( discrete_t ) 1);
  } 
  discrete_t tag = ( discrete_t ) addr >> ( __specs.bits.index + __specs.bits.offset );
  std::tuple search = IsInTheCache( index , tag );
  if( std::get<0>( search ) ) {
    __access.block = std::get<1>( search );
    __access.res = AccessResult::HIT;
    substitutionPolitics.get()->Refresh( index , std::get<1>( search ) );
    __report.hits++;
  } else {
    __report.miss++;
    isFullBlock = IsFullBlock( index );
    isFull = IsFull();
    __access.block = substitutionPolitics.get()->GetBlock( index );

    if( !cache[index][__access.block].val ) {
      __access.res = AccessResult::COMPULSORY_MISS;
      __report.compulsory_miss++;
    } else if( cache[index][__access.block].val && isFull ) {
      __access.res = AccessResult::CAPACITY_MISS;
      __report.capacity_miss++;
    } else {
      __access.res = AccessResult::CONFLICT_MISS;
      __report.conflict_miss++;
    }

    cache[index][__access.block].val = true;
    cache[index][__access.block].tag = tag;
  }
  __access.orig = addr;
  std::cout << __report.accesses << " " << __access.orig << " " << __access.block << " " << index << " " << "\n";
  return this->__access;
}

auto Cache::report() -> CacheReport & {
  __report.Calculate();
  return this->__report;
}

std::tuple< bool , discrete_t > Cache::IsInTheCache( discrete_t index , discrete_t tag ) {
  for ( discrete_t block = 0 ; block < __specs.assoc ; block++ ) {
    if( ( tag == cache[index][block].tag ) && cache[index][block].val ) {
      return std::make_tuple( true , block );
    }
  }
  return std::make_tuple( false , 0 );
}
 
bool Cache::IsFullBlock( discrete_t index ){
  for ( discrete_t block = 0 ; block < __specs.assoc ; block++ ) {
    if( !cache.at( index ).at( block ).val ) {
      return false;
    }
  }
  return true;
}

bool Cache::IsFull(){
  for( discrete_t index = 0 ; index < __specs.nsets ; index++ ) {
    for ( discrete_t block = 0 ; block < __specs.assoc ; block++ ) {
      if( !cache.at( index ).at( block ).val ) {
        return false;
      }
    }
  }
  return true;
}

auto Cache::setCacheSpecs(std::span<std::string> command) -> CacheSpecs {
  return CacheSpecs(32, std::stoull(command[0]), std::stoull(command[1]), std::stoull(command[2]), command[3]);
}
