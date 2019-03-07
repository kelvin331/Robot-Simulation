/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>

# include <sig/gs_array.h>
# include <sig/gs_string.h>
# include <sig/gs_input.h>
# include <sig/gs_mat.h>

void test_random ();
void test_mat ();
void test_matn ();
void test_euler ();
void test_vars ();
void test_heap ();
void test_table ();
void test_slotmap ();
void test_string ();
void test_structures ();
void test_timer ();
void test_array ();
void test_arraylist ();
void test_graph ();
void test_grid ();
void test_list ();

struct FuncDesc { void (*func) (); const char* name; } FD[] =
{	{ test_random,	"random" },
	{ test_timer,	"timer" },
	{ test_string,	"string" },
	{ test_vars,	"vars" },
	{ test_mat,		"mat" },
	{ test_matn,	"matn" },
	{ test_euler,	"euler" },
	{ test_grid,	"grid" },
	{ test_array,	"array" },
	{ test_graph,	"graph" },
	{ test_list,	"list" },
	{ test_heap,	"heap" },
	{ test_table,	"table" },
	{ test_slotmap, "slotmap" },
	{ test_structures, "structures" },
	{ test_arraylist, "arraylist" },
	{ 0, 0 } };

int main ( int argc, char** argv )
{
	int i;
	const char* test = 0; //"arraylist";
	GsString s(test);

	if ( !test )
	{	for ( i=0; FD[i].name; i++ ) gsout<<gspc<<FD[i].name<<gsnl;
		gsout << "\nEnter test name: ";
		s.input();
	}

	if ( s.len()==0 ) return 0;

	for ( i=0; FD[i].name; i++ )
		if (s==FD[i].name) { FD[i].func(); break; }

	if ( !FD[i].name ) { gsout<<"unknown test!\n"; return 0; }

	if ( !gs_console_shown() ) gsout.pause("\nDone.");

	return 0;
}
