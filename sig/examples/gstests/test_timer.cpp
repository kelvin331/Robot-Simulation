/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_output.h>
# include <sig/gs_timer.h>

void test_timer ()
 {
   int i, c;

   GsTimer timer(0);

   for ( c=0; c<20; c++ )
	{ gsout << "Time Now: " << timer.t() << gsnl;
	}

   gsout << "\n5 one second ticks:\n";
   for ( i=1; i<=5; i++ )
	{ c=0; timer.start();
	  do { c++; timer.stop(); } while ( timer.dt()<1.0 );
	  gsout << i <<gspc<< c << " loops, dt=" << timer.dt() <<gsnl;
	}
 }
