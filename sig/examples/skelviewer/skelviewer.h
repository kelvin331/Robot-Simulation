/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
# pragma once

# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_ik_manipulator.h>
# include <sigkin/kn_scene.h>

# include <sigogl/ui_panel.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ui_check_button.h>

# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MySkelViewer : public WsViewer
{  public :
	enum MenuEv { EvInfo, EvView, EvExit, EvP1, EvP2, EvP3, EvR1, EvR2, EvR3, EvPosture=100, EvEff=200, EvJoint=300 };
	SnGroup* _mg;
	KnSkeleton* _sk;
	KnScene* _sksc;
	GsArray<KnJoint*> _effs;
	UiSlider* _sl[6];
	UiCheckButton* _vbut[4];
	UiPanel* _jointsp;
	UiPanel* _valuesp;
	UiPanel* _posturesp;
	UiPanel* _endeffs;
	int _seljoint;

   public :
	MySkelViewer ( KnSkeleton* sk, int x, int y, int w, int h, const char* l );
	void joint_info ( int jid );
	void iksolved ( KnIkManipulator* ikm );
	void build_value_panel ( int jid );
	void update_joint ( int si, int jid );
	virtual int uievent ( int e ) override;
};

