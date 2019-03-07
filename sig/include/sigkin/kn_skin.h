/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_SKIN_H
# define KN_SKIN_H

//================================ KnSkin ===================================

# include <sig/gs_array.h>
# include <sig/sn_model.h>

class SnLines;
class GsModel;
class KnJoint;
class KnSkeleton;

/*! Maintains a model and skinning weights.
	This class is usually owned (via sharing) by a KnSkeleton. */
class KnSkin : public SnModel
{  public :
	struct Weight { KnJoint* j; float w; GsVec v; GsQuat q; };
	struct SkinVtx { int n; Weight* w; };
	GsArray<SkinVtx> SV;
	KnSkeleton* skeleton;
	bool _intn;

   public :
	/*! Constructor  */
	KnSkin ();

	/*! Destructor */
   ~KnSkin ();

	/*! deletes all */
	void init ();

	/*! Init by loading a skin mesh file (.m or .obj) and a skinning weight file (.w).
		Returns false if some error is encountered, otherwise true is returned.
		Parameter basedir can specify where to search for the skin mesh file, 
		and if not given, it is extracted from filename. */
	bool init ( KnSkeleton* skel, const char* filename, const char* basedir=0 );

	/*! Computes the positions of all vertices of the skin according to the weights.
		Will only update if the skin mesh is visible, otherwise nothing is done. */
	void update ();
};


//================================ End of File =================================================

# endif  // KN_SKIN_H
