/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_LIGHT_H
# define GS_LIGHT_H

/** \file gs_light.h 
 * Keeps light parameters
 */

# include <sig/gs_vec.h>
# include <sig/gs_color.h>

//=============================== GsLight ================================

/*! \class GsLight gs_light.h
	\brief Keeps light parameters

	GsLight keeps usual light parameters for a Phong illumination model. */
class GsLight
{  public :
	GsColor ambient;  //!< Default is darkgray
	GsColor diffuse;  //!< Default is white
	GsColor specular; //!< Default is white
	GsVec position;   //!< Default is (0,0,1)

   public :

	/*! Initialize the camera with the default parameters. */
	GsLight ();

	/*! Sets the default parameters. */
	void init ();
 
	/*! Encodes the 3 color intensities of the light as 3 rgb vectors with 
		components in [0,1]. A pointer to vec is then returned. */
	float* encode_intensities ( float vec[9] ) const;
};

//================================ End of File ======================================

# endif // GS_LIGHT_H
