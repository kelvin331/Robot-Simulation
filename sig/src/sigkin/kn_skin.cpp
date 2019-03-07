/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>

# include <sig/sn_model.h>
# include <sigkin/kn_skin.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_joint.h>

//# define KN_USE_TRACE1  // 
# include <sig/gs_trace.h>

//============================= KnSkin ============================

KnSkin::KnSkin ()
 {
 }

KnSkin::~KnSkin ()
 {
   init ();
 }

void KnSkin::init ()
 {
   while ( SV.size()>0 ) delete[] SV.pop().w;
   model()->init();
   skeleton=0;
   _intn=false;
 }

bool KnSkin::init ( KnSkeleton* skel, const char* filename, const char* basedir )
 {
   init ();
   GsInput in;
   GsDirs paths;
   if ( basedir ) paths.basedir ( basedir );
	else paths.basedir_from_filename ( filename );

   // check if there is a mesh skin file to load:
   GsString filest(filename);
   remove_extension(filest);
   filest.append(".m");
   if ( !paths.checkfull(filest) )
	{ remove_extension(filest);
	  filest.append(".obj");
	  if ( !paths.checkfull(filest) ) return false; // no mesh file found
	}

   // load skin mesh:
   if ( !model()->load(filest) ) return false; // error in loading mesh

   // load weights:
   remove_extension(filest);
   filest.append(".w");
   if ( !paths.checkfull(filest) ) return false; // no weights file found
   if ( !in.open(filename) ) return false; // error opening .w file
   in.commentchar ( '#' );
   in.lowercase ( false );

   // check signature, type, etc:
   in.get();
   if ( in.ltoken()=="KnSkinWeights" )
	{ /* ok .w format */ }
   else
	{ gsout.warning("Unknown skinning weights file"); return false; }

   GsQuat q;
   GsMat ml, mi, mv;
   skel->init_values();
   skel->update_global_matrices();
   skeleton = skel;
   _intn = model()->V.size()==model()->N.size();

   while (true)
	{ in.get();
	  if ( in.end() ) break;

	  if ( in.ltype()==GsInput::Number )
	   { 
		 int vid = atoi ( in.ltoken() );
		 if ( vid!=SV.size() ) gsout<<"skin: skin vertex id mismatch\n";
		 int n = in.geti();
		 Weight* w = new Weight[n];
		 SV.push();
		 SV.top().n = n;
		 SV.top().w = w;
		 for ( int i=0; i<n; i++ )
		  { in.get();
			if ( in.ltype()==GsInput::Delimiter ) in.get(); // skip delimiter
			if ( in.ltype()!=GsInput::String ) // missing weight
			 { in.unget(); w[i].j=0; w[i].w=0; gsout<<"skin: missing weight\n"; break; }
			w[i].j = skel->joint(in.ltoken());
			if ( !w[i].j ) gsout<<"skin: unknown joint name: "<<in.ltoken()<<gsnl;
			w[i].w = in.getf();
			if ( w[i].j )
			 { mv.translation ( model()->V[vid] );
			   mi = w[i].j->gmat().inverse();
			   ml.mult ( mv, mi );
			   decompose ( ml, w[i].q, w[i].v );
			   if ( _intn )
				{ //mi.setrans ( GsVec::null );
				  //w[i].n = model()->N[vid] * mi; // put normal in local coords
				  //mv.translation ( model()->N[vid] );
				  /*mat2quat ( w[i].j->gmat(), q );
				  q = (w[i].q * q);
				  w[i].nq = q.inverse();
				  w[i].n = w[i].nq.apply ( model()->N[vid] );*/
				} 
			 }
		  }
	   }
	  else if ( in.ltype()==GsInput::String && in.ltoken()=="end"  )
	   { break;
	   }
	  if ( SV.size()==model()->V.size() ) break;
	}

   SV.compress();
   if ( SV.size()!=model()->V.size() ) gsout.warning("Number of skin vertices differs from skinning weights");
   return true;
 }

void KnSkin::update ()
 {
   if ( !skeleton ) return;
   if ( !visible() ) return;
   skeleton->update_global_matrices();
   GsModel* m = model(); // this will automatically call touch()
   int i, k, size = m->V.size();
   GsPnt vi, wv, ni, wn;
   GsQuat q;

   for ( i=0; i<size; i++ )
	{ Weight* w = SV[i].w;
	  int n = SV[i].n;
	  wn = GsPnt::null;
	  wv = GsPnt::null;
	  for ( k=0; k<n; k++ )
	   { if ( !w[k].j ) break;
		 vi = w[k].v * w[k].j->gmat();
		 wv += vi * w[k].w;
		 if ( _intn )
		  { //mat2quat ( w[k].j->gmat(), q );
			//ni = q.apply(w[k].n);
			//wn += ni * w[k].w;
		  }
	   }
	  m->V[i] = wv;
	  //if ( _intn ) { wn.normalize(); m->N[i] = wn; }
	}
 }

//============================= EOF ===================================
