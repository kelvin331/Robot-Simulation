/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_material.h>
# include <sig/sa_action.h>

//# define GS_USE_TRACE1  // Const/Dest
# include <sig/gs_trace.h>

//======================================= SnTransform ====================================

const char* SnMaterial::class_name = "SnMaterial";

SnMaterial::SnMaterial ()
		    :SnNode ( SnNode::TypeMaterial, SnMaterial::class_name )
{
	GS_TRACE1 ( "Constructor" );
}

SnMaterial::SnMaterial ( const GsMaterial& m )
		   :SnNode ( SnNode::TypeTransform, SnMaterial::class_name )
{
	GS_TRACE1 ( "Constructor from GsMat" );
	_material = m;
}

SnMaterial::~SnMaterial ()
{
	GS_TRACE1 ( "Destructor" );
}

bool SnMaterial::apply ( SaAction* a )
{
	return a->material_apply(this);
}

//======================================= EOF ====================================
