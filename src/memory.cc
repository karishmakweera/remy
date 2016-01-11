#include <boost/functional/hash.hpp>
#include <vector>
#include <cassert>

#include "memory.hh"

using namespace std;

static const double alpha = 1.0 / 8.0;

void Memory::packets_received( const vector< Packet > & packets, const unsigned int flow_id )
{
  for ( const auto &x : packets ) {
    if ( x.flow_id != flow_id ) {
      continue;
    }

    const double rtt = x.tick_received - x.tick_sent;
    if ( _last_tick_sent == 0 || _last_tick_received == 0 ) {
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
      _min_rtt = rtt;
    } else {
      _rec_send_ewma = (1 - alpha) * _rec_send_ewma + alpha * (x.tick_sent - _last_tick_sent);

      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;

      _min_rtt = min( _min_rtt, rtt );
    }
  }
}

string Memory::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "sewma=%f", _rec_send_ewma );
  return tmp;
}

const Memory & MAX_MEMORY( void )
{
  static const Memory max_memory( { 163840, 163840, 163840, 163840 } );
  return max_memory;
}

RemyBuffers::Memory Memory::DNA( void ) const
{
  RemyBuffers::Memory ret;
  ret.set_rec_send_ewma( _rec_send_ewma );
  return ret;
}

/* If fields are missing in the DNA, we want to wildcard the resulting rule to match anything */
#define get_val_or_default( protobuf, field, limit ) \
  ( (protobuf).has_ ## field() ? (protobuf).field() : (limit) ? 0 : 163840 )

Memory::Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna )
  : _rec_send_ewma( get_val_or_default( dna, rec_send_ewma, is_lower_limit ) ),
    _last_tick_sent( 0 ),
    _last_tick_received( 0 ),
    _min_rtt( 0 )
{
}

size_t hash_value( const Memory & mem )
{
  size_t seed = 0;
  boost::hash_combine( seed, mem._rec_send_ewma );
  
  return seed;
}
