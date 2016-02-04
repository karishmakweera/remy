#include "network.hh"

#include "sendergangofgangs.cc"
#include "link-templates.cc"
#include <fstream>
#include <ctime>

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2>::Network( const SenderType1 & example_sender1,
                                            const SenderType2 & example_sender2,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender2, _prng, config.num_senders ) ),
    _link( config.link_ppt, config.buffer_size ),
    _delay( config.delay ),
    _rec(),
    _id(),
    _tickno( 0 )
{
    _id = to_string(example_sender1.whiskers().hash()) + to_string(example_sender2.whiskers().hash()) 
      + to_string(config.hash());
}

template <class SenderType1, class SenderType2>
Network<SenderType1, SenderType2>::Network( const SenderType1 & example_sender1,
                                            PRNG & s_prng,
                                            const NetConfig & config )
  : _prng( s_prng ),
    _senders( SenderGang<SenderType1>( config.mean_on_duration, config.mean_off_duration, config.num_senders, example_sender1, _prng ),
	      SenderGang<SenderType2>() ),
    _link( config.link_ppt, config.buffer_size ),
    _delay( config.delay ),
    _rec(),
    _id(),
    _tickno( 0 )
{
  _id = to_string(example_sender1.whiskers().hash()) + to_string(config.hash());
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::tick( void )
{
  _senders.tick( _link, _rec, _tickno );
  _link.tick( _delay, _tickno );
  _delay.tick( _rec, _tickno );
}

template <class SenderType1, class SenderType2> 
void Network<SenderType1, SenderType2>::export_score(
std::string fname, double last_tick, double duration, bool final_score) 
{  
  std::fstream fs;
  if (final_score) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "100% " << _senders.utility() << "\n";
    fs.close();
  } else if ( _tickno >= duration * 0.01 && last_tick < duration * 0.01 ) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "1% " << _senders.utility() << "\n";
    fs.close();
  } else if ( _tickno >= duration * 0.02 && last_tick < duration * 0.02 ) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "2% " << _senders.utility() << "\n";
    fs.close();
  } else if (_tickno >= duration * 0.04 && last_tick < duration * 0.04 ) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "4% " << _senders.utility() << "\n";
    fs.close();
  } else if (_tickno >= duration * 0.08 && last_tick < duration * 0.08 ) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "8% " << _senders.utility() << "\n";
    fs.close();
  } else if (_tickno >= duration * 0.16 && last_tick < duration * 0.16 ) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "16% " << _senders.utility() << "\n";
    fs.close();
  } else if (_tickno >= duration * 0.32 && last_tick < duration * 0.32) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "32% " << _senders.utility() << "\n";
    fs.close();
  } else if (_tickno >= duration * 0.64 && last_tick < duration * 0.64) {
    fs.open(fname, std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "64% " << _senders.utility() << "\n";
    fs.close();
  }
}

template <class SenderType1, class SenderType2>
void Network<SenderType1, SenderType2>::run_simulation( const double & duration )
{
  assert( _tickno == 0 );
  double last_tick = 0;
  string fname = "../scripts/eval_score/" + to_string(time(0)) + "-" + _id;

  while ( _tickno < duration ) {
    /* find element with soonest event */
    last_tick = _tickno;
    _tickno = min( min( _senders.next_event_time( _tickno ),
			_link.next_event_time( _tickno ) ),
		   min( _delay.next_event_time( _tickno ),
			_rec.next_event_time( _tickno ) ) );

    if ( _tickno > duration ) break;
    assert( _tickno < std::numeric_limits<double>::max() );

    export_score(fname, last_tick, duration);
    
    tick();
  }
  export_score(fname, last_tick, duration, true);
}

