# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	UiCheckButton* _nbut;
	bool _animating;
	float light[4] = { 0,1,1,0 };
	float ground[4] = { 0,1,0,8 };
	GsMat shadowMat;
	GsMat update_shadowMat;
	GsMat floor;
	GsMat lhand,rhand;
	GsMat lmid,rmid;
	GsMat lbase,rbase;
	GsMat lmid_rot, lh_rot;
	GsMat rmid_rot, rh_rot;
	GsMat rleg, rfoot;
	GsMat lleg, lfoot;
	GsMat lfoot_rot, rfoot_rot;
	GsMat lleg_rot, rleg_rot;
	GsMat head, head_rot;
	GsMat body;
	GsMat city;
	GsMat street;
	GsMat Buu;
	GsMat heli;
	GsMat wing;
	GsMat wing_rot;


	SnGroup* e = new SnGroup;//body group
	SnGroup* sh = new SnGroup;// shadow group
	float zinc = 0.0f;
	float xinc = 0.0f;
	float yinc = 0.0f;

	GsMat whole;

	bool cam = true;

   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void rotate(int num, int o);
	void add_model ( SnShape* s, GsVec p );
	void MyViewer::global(int num, int o);
	void MyViewer::follow_view(int num);

	void build_scene ();
	void show_normals ( bool b );
	void run_animation ();
	void MyViewer::add_shadow_model(SnShape* s, GsVec p);
	void MyViewer::camera_view(int num);
	void MyViewer::static_view();
	void MyViewer::moves(int num, int o);
	void MyViewer::circle();
	void MyViewer::car_move(int go);
	void MyViewer::spin();
	void MyViewer::spin2();


	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

