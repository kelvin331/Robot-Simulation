/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "skelviewer.h"

# include <sigogl/ui_dialogs.h>

# include <sigogl/ws_dialog.h>
# include <sigogl/ws_run.h>

int main ( int argc, char **argv )
{
	// Choose a skeleton to load:
	const char* skfile;
	GsStrings s;
	s.push ( "=== load from disk ===" );	// 0
	s.push ( "../data/arms/onearm.s" );		// 1
	s.push ( "../data/arms/twoarm.s" );		// 2
	s.push ( "../data/arms/twolinks.s" );	// 3
	s.push ( "../data/arms/twolinks84.s" );	// 4
	s.push ( "../data/arms/torso.s" );		// 5
	int choice = -1; // change here to start with desired case

	if ( choice>0 )
	{	choice=ui_choice ( "Select skeleton:", s );
		if ( choice<0 )	return 1;
	}

	if ( choice<=0 ) // select file from disk
		skfile = ui_select_file ( "Select skeleton file to load:", "../data/arms/", "*.s;*.bvh" );
	else
		skfile = s[choice];

	// load skeleton:
	if ( !skfile ) return 1;
	KnSkeleton* sk = new KnSkeleton;
	if ( !sk->load(skfile) ) gsout.fatal("could not load %s",skfile);

	// Create and show viewer:
	MySkelViewer* viewer = new MySkelViewer(sk,-1,-1,640,480,"SIG Skelviewer");
	viewer->view_all ();
	viewer->show();

	// let it run:
	ws_run();

	return 1;
}
