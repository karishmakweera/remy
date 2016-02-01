#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>

#include "evaluator.hh"
#include "configrange.hh"
using namespace std;

double parse_outcome(Evaluator::Outcome outcome) {
  double norm_score = 0;
  for ( auto &run : outcome.throughputs_delays ) {
    printf( "===\nconfig: %s\n", run.first.str().c_str() );
    for ( auto &x : run.second ) {
      printf( "sender: [tp=%f, del=%f]\n", x.first / run.first.link_ppt, x.second / run.first.delay );
      norm_score += log2( x.first / run.first.link_ppt ) - log2( x.second / run.first.delay );
    }
  }  
  return norm_score;
}

int main( int argc, char *argv[] )
{
  WhiskerTree whiskers;
  unsigned int num_senders = 2;
  double link_ppt = 1.0;
  double delay = 150.0;
  double mean_on_duration = 1000.0;
  double mean_off_duration = 1000.0;
  int num_points = 100; /* Granularity of the generated plot */
  int signal_index = -1; /* Whether we want to sample signal at given index */
  bool linkspeed = false;
  ofstream plot;

  for ( int i = 1; i < argc; i++ ) {
    string arg( argv[ i ] );
    if ( arg.substr( 0, 3 ) == "if=" ) {
      string filename( arg.substr( 3 ) );
      int fd = open( filename.c_str(), O_RDONLY );
      if ( fd < 0 ) {
	perror( "open" );
	exit( 1 );
      }

      RemyBuffers::WhiskerTree tree;
      if ( !tree.ParseFromFileDescriptor( fd ) ) {
	fprintf( stderr, "Could not parse %s.\n", filename.c_str() );
	exit( 1 );
      }
      whiskers = WhiskerTree( tree );

      if ( close( fd ) < 0 ) {
	perror( "close" );
	exit( 1 );
      }

      if ( tree.has_config() ) {
	printf( "Prior assumptions:\n%s\n\n", tree.config().DebugString().c_str() );
      }

      if ( tree.has_optimizer() ) {
	printf( "Remy optimization settings:\n%s\n\n", tree.optimizer().DebugString().c_str() );
      }
    } else if ( arg.substr( 0, 5 ) == "nsrc=" ) {
      num_senders = atoi( arg.substr( 5 ).c_str() );
      fprintf( stderr, "Setting num_senders to %d\n", num_senders );
    } else if ( arg.substr( 0, 5 ) == "link=" ) {
      link_ppt = atof( arg.substr( 5 ).c_str() );
      fprintf( stderr, "Setting link packets per ms to %f\n", link_ppt );
    } else if ( arg.substr( 0, 4 ) == "rtt=" ) {
      delay = atof( arg.substr( 4 ).c_str() );
      fprintf( stderr, "Setting delay to %f ms\n", delay );
    } else if ( arg.substr( 0, 3 ) == "on=" ) {
      mean_on_duration = atof( arg.substr( 3 ).c_str() );
      fprintf( stderr, "Setting mean_on_duration to %f ms\n", mean_on_duration );
    } else if ( arg.substr( 0, 4 ) == "off=" ) {
      mean_off_duration = atof( arg.substr( 4 ).c_str() );
      fprintf( stderr, "Setting mean_off_duration to %f ms\n", mean_off_duration );
    } else if ( arg.substr( 0, 6 ) == "granu=") {
      num_points = atof( arg.substr( 6 ).c_str() );
      fprintf( stderr, "Setting granularity to %d points\n", num_points );
    } else if ( arg.substr( 0, 7 ) == "export=") {
      signal_index = atof( arg.substr( 7 ).c_str() );
    } else if ( arg.substr( 0, 10 ) == "linkspeed=") {
      string filename( arg.substr( 10 ) );
      plot.open(filename);
      linkspeed = true;
      fprintf( stderr, "Outputting to %s\n", filename.c_str() );
    }
   }

  ConfigRange configuration_range;
  configuration_range.link_ppt = Range( link_ppt,link_ppt, 0 );
  configuration_range.rtt = Range( delay, delay, 0 ); /* ms */
  configuration_range.num_senders = Range(num_senders, num_senders, 0 );
  configuration_range.mean_on_duration = Range(mean_on_duration, mean_on_duration, 0);
  configuration_range.mean_off_duration = Range(mean_off_duration, mean_off_duration, 0);
  if (linkspeed) {
    /* Link speed range we are evaluating is 1 to 1000 Mbps. */
    link_ppt = 0.1;
    double step = pow(10, 3.0 / (num_points - 1));
    for (int i = 0; i < num_points; i ++) {
      configuration_range.link_ppt = Range( link_ppt,link_ppt, 0 );
      Evaluator eval( configuration_range, signal_index );
      auto outcome = eval.score( whiskers, false, 10 );
      double norm_score = parse_outcome(outcome);
      plot << to_string(link_ppt * 10) + " ";
      plot << to_string(norm_score / 2) + "\n";
      link_ppt *= step;
    }
    plot.close();
  } else {
    Evaluator eval( configuration_range, signal_index );
    auto outcome = eval.score( whiskers, false, 10 );
    parse_outcome(outcome);  
  }
  
  return 0;
}

