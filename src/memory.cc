#include <boost/functional/hash.hpp>
#include <vector>
#include <cassert>

#include "memory.hh"

using namespace std;

static const double alpha = 1.0 / 8.0;

void Memory::packets_received( const vector< Packet > & packets, const unsigned int flow_id, 
  const int largest_ack )
{
  if (packets.size() > 1) {
    printf("packets received: %ld\n", packets.size());
  }
  for ( const auto &x : packets ) {
    if ( x.flow_id != flow_id ) {
      continue;
    }

    int pkt_outstanding = 1;
    if ( x.seq_num > largest_ack ) {
      pkt_outstanding = x.seq_num - largest_ack;
    }
    if ( _last_tick_sent == 0 || _last_tick_received == 0 ) {
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
    } else {
      _rec_ewma = (1 - alpha) * _rec_ewma + alpha * (x.tick_received - _last_tick_received);
      _queueing_delay = _rec_ewma * pkt_outstanding;
      _last_tick_sent = x.tick_sent;
      _last_tick_received = x.tick_received;
    }
  }
}

string Memory::str( void ) const
{
  char tmp[ 256 ];
  snprintf( tmp, 256, "queueing_delay=%f", _queueing_delay );
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
  ret.set_queueing_delay( _queueing_delay );
  return ret;
}

/* If fields are missing in the DNA, we want to wildcard the resulting rule to match anything */
#define get_val_or_default( protobuf, field, limit ) \
  ( (protobuf).has_ ## field() ? (protobuf).field() : (limit) ? 0 : 163840 )

Memory::Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna )
  : _queueing_delay( get_val_or_default( dna, queueing_delay, is_lower_limit ) ),
    _last_tick_sent( 0 ),
    _last_tick_received( 0 ),
    _rec_ewma( 0 )
{
}

size_t hash_value( const Memory & mem )
{
  size_t seed = 0;
  boost::hash_combine( seed, mem._queueing_delay );
  
  return seed;
}
