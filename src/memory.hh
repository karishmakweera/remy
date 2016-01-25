#ifndef MEMORY_HH
#define MEMORY_HH

#include <vector>
#include <string>

#include "packet.hh"
#include "dna.pb.h"

class Memory {
public:
  typedef double DataType;

private:
  DataType _queueing_delay;
  
  double _last_tick_sent;
  double _last_tick_received;
  double _rec_ewma;

public:
  Memory( const std::vector< DataType > & s_data )
    : _queueing_delay( s_data.at( 0 ) ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 ),
      _rec_ewma( 0 )
  {}

  Memory()
    : _queueing_delay( 0.0 ),
      _last_tick_sent( 0 ),
      _last_tick_received( 0 ),
      _rec_ewma( 0 )
  {}

  void reset( void ) { _queueing_delay = _last_tick_sent = _last_tick_received = _rec_ewma = 0; }

  static const unsigned int datasize = 1;

  const DataType & field( unsigned int num ) const { return num == 0 ? _queueing_delay : _queueing_delay ; }
  DataType & mutable_field( unsigned int num )     { return num == 0 ? _queueing_delay : _queueing_delay ;  }

  void packet_sent( const Packet & packet __attribute((unused)) ) {}
  void packets_received( const std::vector< Packet > & packets, const unsigned int flow_id, const int largest_ack );
  void advance_to( const unsigned int tickno __attribute((unused)) ) {}

  std::string str( void ) const;

  bool operator>=( const Memory & other ) const { return (_queueing_delay >= other._queueing_delay) ; }
  bool operator<( const Memory & other ) const { return (_queueing_delay < other._queueing_delay) ; }
  bool operator==( const Memory & other ) const { return (_queueing_delay == other._queueing_delay) ; }

  RemyBuffers::Memory DNA( void ) const;
  Memory( const bool is_lower_limit, const RemyBuffers::Memory & dna );

  friend size_t hash_value( const Memory & mem );
};

extern const Memory & MAX_MEMORY( void );

#endif
