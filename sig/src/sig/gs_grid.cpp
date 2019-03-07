/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_grid.h>

//============================ GsGridBase =================================

GsGridBase::GsGridBase ( int dim, int ns )
 {
   _cells = 0;
   if ( dim*ns>0 ) init ( dim, ns );
 } 

void GsGridBase::init ( int dim, int ns )
 {
   int i;
   _axis.size ( dim );

   for ( i=0; i<dim; i++ )
	{ _axis[i].min = 0.0f;
	  _axis[i].max = 1.0f;
	  _axis[i].segs = ns;
	}

   init ( _axis );
 } 

void GsGridBase::init ( const GsArray<GsGridAxis>& axis_desc )
 {
   if ( &axis_desc != &_axis ) _axis=axis_desc;
   
   if ( _axis.size()==0 ) { _cells=0; _size.size(0); _seglen.size(0); return; }

   int i, j;

   _cells = _axis[0].segs;
   int dim = _axis.size();

   for ( i=1; i<dim; i++ )
	{ _cells *= _axis[i].segs;
	}

   _size.size ( dim );
   for ( i=0; i<dim; i++ )
	{ _size[i] = 1;
	  for ( j=0; j<i; j++ ) _size[i]*=_axis[j].segs;
	}

   _seglen.size ( dim );
   for ( i=0; i<dim; i++ )
	_seglen[i] = (_axis[i].max-_axis[i].min)/float(_axis[i].segs);
 }

int GsGridBase::cell_index ( const GsArray<int>& coords ) const
 {
   int dim = _axis.size();

   if ( dim!=coords.size() || dim<=0 ) return 0;
   
   int i, cell = coords[0];
   for ( i=1; i<dim; i++ ) cell += coords[i]*_size[i];

   return cell;
 }

void GsGridBase::cell_coords ( int index, int& i, int& j ) const
 {
   if ( dimensions()!=2 ) { i=j=0; return; }
   
   j = index/_size[1]; index=index%_size[1];
   i = index;
 }

void GsGridBase::cell_coords ( int index, int& i, int& j, int& k ) const
 {
   if ( dimensions()!=3 ) { i=j=k=0; return; }
   
   k = index/_size[2]; index=index%_size[2];
   j = index/_size[1]; index=index%_size[1];
   i = index;
 }

void GsGridBase::cell_coords ( int index, GsArray<int>& coords ) const
 {
   int d = dimensions();

   if ( coords.size()!=d ) coords.size(d);

   d--;
   while ( d>0 )
	{ coords[d] = index/_size[d];
	  index = index%_size[d];
	  d--;
	}
   coords[0] = index;
 }

void GsGridBase::cell_boundary ( int i, int j, GsPnt2& a, GsPnt2& b ) const
 {
   if ( dimensions()!=2 ) return;
   
   a.x = _axis[0].min + float(i)*_seglen[0];
   a.y = _axis[1].min + float(j)*_seglen[1];
   
   b.x = a.x + _seglen[0];
   b.y = a.y + _seglen[1];
 }

void GsGridBase::cell_boundary ( int i, int j, int k, GsPnt& a, GsPnt& b ) const
 {
   if ( dimensions()!=3 ) return;
   
   a.x = _axis[0].min + float(i)*_seglen[0];
   a.y = _axis[1].min + float(j)*_seglen[1];
   a.z = _axis[2].min + float(k)*_seglen[2];
   
   b.x = a.x + _seglen[0];
   b.y = a.y + _seglen[1];
   b.z = a.z + _seglen[2];
 }

void GsGridBase::get_intersection ( GsPnt2 a, GsPnt2 b, GsArray<int>& cells ) const
 {
   if ( dimensions()!=2 ) return;

   if ( a.x>b.x || a.y>b.y ) return;

   if ( a.x>=_axis[0].max || a.y>=_axis[1].max ) return;
   if ( b.x<=_axis[0].min || b.y<=_axis[1].min ) return;

   float f = _seglen[0]/2;
   a.x = GS_BOUND ( a.x, _axis[0].min, _axis[0].max-f );
   b.x = GS_BOUND ( b.x, _axis[0].min, _axis[0].max-f );
   f = _seglen[1]/2;
   a.y = GS_BOUND ( a.y, _axis[1].min, _axis[1].max-f );
   b.y = GS_BOUND ( b.y, _axis[1].min, _axis[1].max-f );
   
   int i1 = (int) ((a.x-_axis[0].min) / _seglen[0]);
   int j1 = (int) ((a.y-_axis[1].min) / _seglen[1]);

   f = (b.x-_axis[0].min) / _seglen[0];
   int i2 = int(f); if ( float(i2)==f ) i2--;

   f = (b.y-_axis[1].min) / _seglen[1];
   int j2 = int(f); if ( float(j2)==f ) j2--;

   int i, j, index;

   for ( j=j1; j<=j2; j++ )
	{ index = _size[1]*j + i1; // == cell_index(i1,j);
	  for ( i=i1; i<=i2; i++ )
	   { cells.push() = index;
		 index++;
	   }
	}
 }

void GsGridBase::get_intersection ( GsPnt a, GsPnt b, GsArray<int>& cells ) const
 {
   if ( dimensions()!=3 ) return;

   if ( a.x>b.x || a.y>b.y || a.z>b.z ) return;

   if ( a.x>=_axis[0].max || a.y>=_axis[1].max || a.z>=_axis[2].max ) return;
   if ( b.x<=_axis[0].min || b.y<=_axis[1].min || b.z<=_axis[2].min ) return;

   float f = _seglen[0]/2;
   a.x = GS_BOUND ( a.x, _axis[0].min, _axis[0].max-f );
   b.x = GS_BOUND ( b.x, _axis[0].min, _axis[0].max-f );
   f = _seglen[1]/2;
   a.y = GS_BOUND ( a.y, _axis[1].min, _axis[1].max-f );
   b.y = GS_BOUND ( b.y, _axis[1].min, _axis[1].max-f );
   f = _seglen[2]/2;
   a.z = GS_BOUND ( a.z, _axis[2].min, _axis[2].max-f );
   b.z = GS_BOUND ( b.z, _axis[2].min, _axis[2].max-f );
   
   int i1 = (int) ((a.x-_axis[0].min) / _seglen[0]);
   int j1 = (int) ((a.y-_axis[1].min) / _seglen[1]);
   int k1 = (int) ((a.z-_axis[2].min) / _seglen[2]);

   f = (b.x-_axis[0].min) / _seglen[0];
   int i2 = int(f); if ( float(i2)==f ) i2--;

   f = (b.y-_axis[1].min) / _seglen[1];
   int j2 = int(f); if ( float(j2)==f ) j2--;

   f = (b.z-_axis[2].min) / _seglen[2];
   int k2 = int(f); if ( float(k2)==f ) k2--;

   int i, j, k, index;

   for ( k=k1; k<=k2; k++ )
	for ( j=j1; j<=j2; j++ )
	 { index = _size[2]*k + _size[1]*j + i1; // == cell_index(i1,j,k);
	   for ( i=i1; i<=i2; i++ )
		{ cells.push() = index;
		  index++;
		}
	 }
 }

int GsGridBase::get_point_location ( GsPnt2 a ) const
 {
   if ( dimensions()!=2 ) return -1;

   if ( a.x>=_axis[0].max || a.y>=_axis[1].max ) return -1;
   if ( a.x<=_axis[0].min || a.y<=_axis[1].min ) return -1;

   a.x = GS_BOUND ( a.x, _axis[0].min, _axis[0].max-(_seglen[0]/2) );
   a.y = GS_BOUND ( a.y, _axis[1].min, _axis[1].max-(_seglen[1]/2) );
   
   int i = (int) ((a.x-_axis[0].min) / _seglen[0]);
   int j = (int) ((a.y-_axis[1].min) / _seglen[1]);

   return _size[1]*j + i;
 }

int GsGridBase::get_point_location ( GsPnt a ) const
 {
   if ( dimensions()!=3 ) return -1;

   if ( a.x>=_axis[0].max || a.y>=_axis[1].max || a.z>=_axis[2].max ) return -1;
   if ( a.x<=_axis[0].min || a.y<=_axis[1].min || a.z<=_axis[2].min ) return -1;

   a.x = GS_BOUND ( a.x, _axis[0].min, _axis[0].max-(_seglen[0]/2) );
   a.y = GS_BOUND ( a.y, _axis[1].min, _axis[1].max-(_seglen[1]/2) );
   a.z = GS_BOUND ( a.z, _axis[2].min, _axis[2].max-(_seglen[2]/2) );
   
   int i = (int) ((a.x-_axis[0].min) / _seglen[0]);
   int j = (int) ((a.y-_axis[1].min) / _seglen[1]);
   int k = (int) ((a.z-_axis[2].min) / _seglen[2]);

   return _size[2]*k + _size[1]*j + i;
 }

//============================== end of file ===============================

