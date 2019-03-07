/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_MATERIAL
# define SN_MATERIAL

/** \file sn_material.h
 * material information to be used by the next shape node
 */

# include <sig/gs_material.h>
# include <sig/sn_node.h>

//======================================= SnMaterial ====================================

/*! \class SnMaterial sn_material.h
	\brief material information to be used by the next shape node

	SnMaterial stores material information to be used by the next shape node, allowing
	different materials to be used by a same shared SnShape during rendering traversal. */
class SnMaterial : public SnNode
 { private :
	GsMaterial _material;

   public :
	static const char* class_name; //<! Contains string SnMaterial

   protected :
	/*! Destructor only accessible through unref() */
	virtual ~SnMaterial ();

   public :
	/*! Default constructor */
	SnMaterial ();

	/*! Constructor receiving a material */
	SnMaterial ( const GsMaterial& m );

	/*! Sets the material to be used for the shape */
	void material ( const GsMaterial& m ) { _material=m; }
	const GsMaterial& material () const { return _material; }

   protected :

	/*! Calls a->transform_apply() for this node */
	virtual bool apply ( SaAction* a ) override;
};

//================================ End of File =================================================

# endif  // SN_MATERIAL
