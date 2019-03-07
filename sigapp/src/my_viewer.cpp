
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>
# include <sigogl/ws_run.h>
GsMat tran;
float pi = 3.14f;
float xcam=0;
float zcam=0;
float c = 0;
int b = 0;
bool fly = false;
int lmid_count = 0;
int lhand_count = 0;
int rmid_count = 0;
int rhand_count = 0;
int lfoot_count = 0;
int rfoot_count = 0;
int lleg_count = 0;
int rleg_count = 0;
int arm_move = 0;
int arm_move1 = 0;
int arm_move2= 0;
int arm_move3 = 0;

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
	cmd(WsViewer::VCmdAsIs);
}
void MyViewer::car_move(int go) {
	GsMat cgo;
	if (go == 0) {
		cgo.translation(GsVec(7, 0, 0));
	}
	else if (go == 1) {
		cgo.translation(GsVec(-7, 0, 0));
	}

	SnManipulator* cg = e->get<SnManipulator>(18);
	cg->initial_mat(cgo*cg->mat());
}

void MyViewer::moves(int num, int o) {
	if (num == 11 && o == 1) {
		for (int ind = 0; ind <= 11; ind++) {
			GsMat whole;
			whole.roty((pi) / 2);
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(0, 0, 0);
			manip12->initial_mat((whole*manip12->mat()));
		}
	}
	if (num == 12 && o == 1) {
		for (int ind = 0; ind <= 11; ind++) {
			GsMat whole;
			c += pi / 2;
			whole.roty(c);
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			manip12->initial_mat(manip12->mat()*whole);
		}
	}
	if (num == 12 && o == -1) {
		for (int ind = 0; ind <= 11; ind++) {
			GsMat whole;
			whole.roty(-pi / 2);
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc, 0, zinc + .4f);
			manip12->initial_mat((whole*manip12->mat()));
		}
	}
}
void MyViewer::circle(){
	//fly = !fly;
	SnManipulator* manip = e->get<SnManipulator>(15);
	GsMat c_cir;
	float inc = pi / 500;
	//do {
		GsMat t1, t2 = manip->mat();
		t1.roty(-pi / 100);
		manip->initial_mat(t1*t2);
		for (float t = 0; t > 2 * pi; t += inc) {
			float x = float(cos(t));
			float y = float(sin(t));
			c_cir.translation(GsVec( x,  y, 0));
		}
		render();
		ws_check();
//	} while (fly);
	

	
}
void MyViewer::global(int num, int o) {
	if (num == 11 && o == 1) {
		run_animation();
	}
	if (num == 12&&o==1) {
		for (int ind = 0; ind <= 11; ind++) {
			GsMat whole;
			whole.roty(pi / 2);
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc, 0, zinc+.4f);
			manip12->initial_mat((tran*manip12->mat()));
		}
		//run_animation();
		//gsout << arm_move;
	}
	if (num == 12 && o == -1) {
		for (int ind = 0; ind <= 11; ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc, 0, zinc-.4f);
			manip12->initial_mat(tran*manip12->mat());
		}
	}
	if (num == 13&&o==1) {
		c+=pi/2;
		b++;
		whole.roty(c);
		//float rote = pi / 2;
		for (int ind = 0; ind <= 11; ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc+.4f, 0, zinc);
			manip12->initial_mat(tran*manip12->mat());
			//manip12->initial_mat(tran*whole);
		}
	}
	if (num == 13 && o == -1) {
		for (int ind = 0; ind <= 11; ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc-.4f, 0, zinc);
			manip12->initial_mat(tran*manip12->mat());
		}
		
	}
	if (num == 14 && o == 1) {
		for (int ind = 0; ind <= 11; ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc , yinc+.1f, zinc);
			manip12->initial_mat(tran*manip12->mat());
		}

	}
	if (num == 14 && o == -1) {
		for (int ind = 0; ind <= 11; ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc, yinc - .1f, zinc);
			manip12->initial_mat(tran*manip12->mat());
		}

	}
	if (num == 15 && o == 1) {
			SnManipulator* manip12 = e->get<SnManipulator>(15); // access one of the manipulators
			tran.translation(xinc, yinc + .1f, zinc);
			manip12->initial_mat(tran*manip12->mat());
		

	}
	if (num == 15 && o == -1) {
			SnManipulator* manip12 = e->get<SnManipulator>(15); // access one of the manipulators
			tran.translation(xinc, yinc - .1f, zinc);
			manip12->initial_mat(tran*manip12->mat());
		

	}
	if (num == 16 && o == 1) {
		c += pi / 2;
		b++;
		whole.roty(c);
		//float rote = pi / 2;
			SnManipulator* manip12 = e->get<SnManipulator>(15); // access one of the manipulators
			tran.translation(xinc + .4f, 0, zinc);
			manip12->initial_mat(tran*manip12->mat());
			//manip12->initial_mat(tran*whole);
		
	}
	if (num == 16 && o == -1) {
		c += pi / 2;
		b++;
		whole.roty(c);
		SnManipulator* manip12 = e->get<SnManipulator>(15); // access one of the manipulators
		tran.translation(xinc - .4f, 0, zinc);
		manip12->initial_mat(tran*manip12->mat());

	}
	if (num == 17 && o == 1) {
		for (int ind = 16; ind <= 17;ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc, yinc + .1f, zinc);
			manip12->initial_mat(tran*manip12->mat());
		}

	}
	if (num == 17 && o == -1) {
		for (int ind = 16; ind <= 17; ind++) {
			SnManipulator* manip12 = e->get<SnManipulator>(ind); // access one of the manipulators
			tran.translation(xinc, yinc - .1f, zinc);
			manip12->initial_mat(tran*manip12->mat());
		}

	}
}
void MyViewer::rotate(int num, int o) {
	SnManipulator* manip = e->get<SnManipulator>(2); // access one of the manipulators
	SnManipulator* manip2 = e->get<SnManipulator>(3); // access one of the manipulators
	SnManipulator* manip3 = e->get<SnManipulator>(4); // access one of the manipulators
	SnManipulator* manip4 = e->get<SnManipulator>(5); // access one of the manipulators
	SnManipulator* manip5 = e->get<SnManipulator>(6); // access one of the manipulators
	SnManipulator* manip6 = e->get<SnManipulator>(7); // access one of the manipulators
	SnManipulator* manip7 = e->get<SnManipulator>(8); // access one of the manipulators
	SnManipulator* manip8 = e->get<SnManipulator>(9); // access one of the manipulators
	SnManipulator* manip9 = e->get<SnManipulator>(10); // access one of the manipulators
	SnManipulator* manip10 = e->get<SnManipulator>(11); // access one of the manipulators
	SnManipulator* manip11 = e->get<SnManipulator>(0); // access one of the manipulators
	SnManipulator* manip12 = e->get<SnManipulator>(15); // access one of the manipulators
	SnManipulator* manip13 = e->get<SnManipulator>(16); // access one of the manipulators
	SnManipulator* manip14 = e->get<SnManipulator>(17); // access one of the manipulators

	GsMat rotate;
	GsMat rotate2;

	rotate.rotx(-pi / 72 * o);
	rotate2.roty(-pi / 72 * o);

	lmid_rot.rotx((-pi / 72)*lmid_count);
	lh_rot.rotx((-pi / 72)*lhand_count);
	rmid_rot.rotx((-pi / 72)*rmid_count);
	rh_rot.rotx((-pi / 72)*rhand_count);
	lfoot_rot.rotx((-pi / 72)*lfoot_count);
	rfoot_rot.rotx((-pi / 72)*rfoot_count);
	lleg_rot.rotx((-pi / 72)*lleg_count);
	rleg_rot.rotx((-pi / 72)*rleg_count);
	wing_rot.roty((-pi / 72));


	if (num == 3) {

		manip->initial_mat((manip->mat()*rotate));
		manip2->initial_mat(manip->mat()*lmid*lmid_rot);
		manip3->initial_mat((manip2->mat()*lhand*lh_rot));
		//arm_move++;
		//gsout << arm_move;
	}
	if (num == 2) {

		manip2->initial_mat((manip2->mat()*rotate));
		manip3->initial_mat(manip2->mat()*lhand*lh_rot);
		lmid_count += 1 * o;


	}
	if (num == 1) {

		manip3->initial_mat((manip3->mat()*rotate));
		lhand_count += 1 * o;
	}
	

	if (num == 6) {


		manip4->initial_mat((manip4->mat()*rotate));
		manip5->initial_mat(manip4->mat()*rmid*rmid_rot);
		manip6->initial_mat((manip5->mat()*rhand*rh_rot));
	}
	if (num == 5) {


		manip5->initial_mat((manip5->mat()*rotate));
		manip6->initial_mat(manip5->mat()*rhand*rh_rot);
		
		rmid_count += 1 * o;
	}
	if (num == 4) {

		manip6->initial_mat((manip6->mat()*rotate));
		rhand_count += 1 * o;


	}
	if (num == 8) {


		manip7->initial_mat((manip7->mat()*rotate));
		manip8->initial_mat(manip7->mat()*rfoot*rfoot_rot);

		
		rleg_count += 1 * o;


	}
	if (num == 7) {


		manip8->initial_mat((manip8->mat()*rotate));
		rfoot_count += 1 * o;
	}

	if (num == 10) {


		manip9->initial_mat((manip9->mat()*rotate));
		manip10->initial_mat(manip9->mat()*lfoot*lfoot_rot);

		
		lleg_count += 1 * o;


	}
	if (num == 9) {


		manip10->initial_mat((manip10->mat()*rotate));

		lfoot_count += 1 * o;
	}
	if (num == 11) {

		head_rot.roty(-pi / 72*o);
		manip11->initial_mat((manip11->mat()*head_rot));

	}	
	if (num == 12) {

		
		manip12->initial_mat((manip12->mat()*rotate));

	}
	if (num == 14) {


		//manip13->initial_mat((manip13->mat()*rotate2));
		manip14->initial_mat(manip14->mat()*wing_rot);
	}


	if (lmid_count == 144) {
		lmid_count = 0;

	}
	if (lmid_count == 0) {
		lmid_count = 144;
	}
	if (lhand_count == 144) {
		lhand_count = 0;

	}
	if (lhand_count == 0) {
		lhand_count = 144;
	}
	//for right arm

	if (rmid_count == 144) {
		rmid_count = 0;

	}
	if (rmid_count == 0) {
		rmid_count = 144;
	}
	if (rhand_count == 144) {
		rhand_count = 0;

	}
	if (rhand_count == 0) {
		rhand_count = 144;
	}
	//right leg

	if (rfoot_count == 144) {
		rfoot_count = 0;

	}
	if (rfoot_count == 0) {
		rfoot_count = 144;
	}
	if (rleg_count == 144) {
		rleg_count = 0;

	}
	if (rleg_count == 0) {
		rleg_count = 144;
	}
	//left leg

	if (lfoot_count == 144) {
		lfoot_count = 0;

	}
	if (lfoot_count == 0) {
		lfoot_count = 144;
	}
	if (lleg_count == 144) {
		lleg_count = 0;

	}
	if (lleg_count == 0) {
		lleg_count = 144;
	}

}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	//new SnTransform
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);
	manip->visible(false);
	e->add(manip);
}
void MyViewer::spin() {

	SnManipulator* round = e->get<SnManipulator>(19);
	GsMat pos = round->mat();
	GsMat rot = round->mat();

	rot.roty(float(-pi / 10));
	round->initial_mat(pos * rot);

	render();
	ws_check();
}
void MyViewer::spin2() {

	SnManipulator* round = e->get<SnManipulator>(20);
	GsMat pos = round->mat();
	GsMat rot = round->mat();

	rot.roty(float(pi / 10));
	round->initial_mat(pos * rot);

	render();
	ws_check();
}
void MyViewer::follow_view(int num) {//follows my robot
		if (num==0) {
			zcam += .4f;
		}
		if (num==1) {
			zcam -= .4f;
		}
		if (num == 2) {
			xcam += .4f;
		}
		if (num == 3) {
			xcam -= .4f;
		}
		
		//camera().eye.z = 0.01f+zcam;
		camera().eye.x = .01f+xcam;
		camera().eye.y = 60.0f;
		camera().center.z = 0+zcam;
		camera().center.x = 0+xcam;
		camera().center.y = 1.0f;
		
		ws_check();
}
void MyViewer::camera_view(int num) {// camrera view that flips wiggles and moves on the z and x axis
	if (num == 0) {
		zcam += .4f;
	}
	if (num == 1) {
		zcam -= .4f;
	}
	if (num == 2) {
		xcam += .4f;
	}
	if (num == 3) {
		xcam -= .4f;
	}
	camera().center.x = xcam;
	camera().center.y = 3.0f;
	camera().center.z = 3.0f+zcam;

	camera().eye.x = xcam;
	camera().eye.y = 3.0f;
	camera().eye.z = 1.0f+zcam;
	render();
	ws_check();
		} 
void MyViewer::static_view() {//static camera view
	camera().up.x = 0;
	camera().up.y = 1;
	camera().up.z = 1;
	render();
	ws_check();
}

void MyViewer::add_shadow_model(SnShape* s, GsVec p)
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	//new SnTransform (maybe use this another day)
	m.translation(p);
	manip->initial_mat(m);

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	sh->add(manip);
	//make new group for shadow in add shadow function
}

void MyViewer::build_scene ()
{
	lmid_count = 0;
	lhand_count = 0;
	rmid_count = 0;
	rhand_count = 0;
	lfoot_count = 0;
	rfoot_count = 0;
	
	
	SnManipulator* r = new SnManipulator;
	r->visible(false);
	tran.translation(0, 0, 0);
	r->initial_mat(tran);
	r->child(e);
	rootg()->add(r);
	
	SnManipulator* rs = new SnManipulator;
	rs->visible(false);
	tran.translation(0, 0, 0);
	rs->initial_mat(tran);
	rs->child(sh);
	rootg()->add(rs);
	
	GsMaterial shadowMaterial;//sets materials to black
	shadowMaterial.init(GsColor::black, GsColor::black, GsColor::black, GsColor::black,0.0f);
	
	GsMat scale;
	scale.scaling(300.5f);

	GsModel *show = new GsModel;
	show->load("..\\robot\\head.obj");
	SnModel	*z = new SnModel(show);
	add_model(z, GsVec(0, 2.5f, 0));
	SnManipulator* manip = e->get<SnManipulator>(0); // access one of the manipulators
	manip->visible(false);
	manip = e->get<SnManipulator>(0); // access one of the manipulators
	head = manip->mat();

	GsModel *show2 = new GsModel;
	show2->load("..\\robot\\body.obj");
	SnModel	*z2 = new SnModel(show2);
	add_model(z2, GsVec(0, 0, 0));
	SnManipulator* manip2 = e->get<SnManipulator>(1); // access one of the manipulators
	manip2->visible(false);
	body = manip2->mat();
	
	GsModel *show3 = new GsModel;
	show3->load("..\\robot\\left shoulder.obj");
	SnModel	*z3 = new SnModel(show3);
	add_model(z3, GsVec(1.8f, 2.5f, 0));
	SnManipulator* manip3 = e->get<SnManipulator>(2); // access one of the manipulators
	manip3->visible(false);
	lbase = manip3->mat();
	
	GsModel *show4 = new GsModel;
	show4->load("..\\robot\\left arm.obj");
	SnModel	*z4 = new SnModel(show4);
	add_model(z4, GsVec(1.8f, 0, 0));
	SnManipulator* manip4 = e->get<SnManipulator>(3); // access one of the manipulators
	manip4->visible(false);
	lmid = manip4->mat();
	lmid = lmid *lbase.inverse();
	
	GsModel *show5 = new GsModel;
	show5->load("..\\robot\\left hand.obj");
	SnModel	*z5 = new SnModel(show5);
	add_model(z5, GsVec(1.8f, -1.8f, 0));
	SnManipulator* manip5 = e->get<SnManipulator>(4); // access one of the manipulators
	manip5->visible(false);
	lhand = manip5->mat();
	lhand = lhand * lmid.inverse()*lbase.inverse();
	
	GsModel *show6 = new GsModel;
	show6->load("..\\robot\\right shoulder.obj");
	SnModel	*z6 = new SnModel(show6);
	add_model(z6, GsVec(-1.8f, 2.5f, 0));
	SnManipulator* manip6 = e->get<SnManipulator>(5); // access one of the manipulators
	manip6->visible(false);
	//manip6 = e->get<SnManipulator>(10); // access one of the manipulators
	rbase = manip6->mat();

	GsModel *show7 = new GsModel;
	show7->load("..\\robot\\right arm.obj");
	SnModel	*z7 = new SnModel(show7);
	add_model(z7, GsVec(-1.8f, 0, 0));
	SnManipulator* manip7 = e->get<SnManipulator>(6); // access one of the manipulators
	manip7->visible(false);
	rmid = manip7->mat();
	rmid = rmid * rbase.inverse();
	
	GsModel *show8 = new GsModel;
	show8->load("..\\robot\\right hand.obj");
	SnModel	*z8 = new SnModel(show8);
	add_model(z8, GsVec(-1.8f, -1.8f, 0));
	SnManipulator* manip8 = e->get<SnManipulator>(7); // access one of the manipulators
	manip8->visible(false);
	rhand = manip8->mat();
	rhand = rhand * rmid.inverse()*rbase.inverse();
	
	GsModel *show9 = new GsModel;
	show9->load("..\\robot\\right leg.obj");
	SnModel	*z9 = new SnModel(show9);
	add_model(z9, GsVec(-.8f, -1.5f, 0));
	SnManipulator* manip9 = e->get<SnManipulator>(8); // access one of the manipulators
	manip9->visible(false);
	rleg = manip9->mat();

	GsModel *show10 = new GsModel;
	show10->load("..\\robot\\right foot.obj");
	SnModel	*z10 = new SnModel(show10);
	add_model(z10, GsVec(-.8f, -3.5f, 0));
	SnManipulator* manip10 = e->get<SnManipulator>(9); // access one of the manipulators
	manip10->visible(false);
	rfoot = manip10->mat();
	rfoot = rfoot*rleg.inverse();
	
	GsModel *show11 = new GsModel;
	show11->load("..\\robot\\left leg.obj");
	SnModel	*z11 = new SnModel(show11);
	add_model(z11, GsVec(.8f, -1.5f, 0));
	SnManipulator* manip11 = e->get<SnManipulator>(10); // access one of the manipulators
	manip11->visible(false);
	lleg = manip11->mat();
	
	GsModel *show12 = new GsModel;
	show12->load("..\\robot\\left foot.obj");
	SnModel	*z12 = new SnModel(show12);
	add_model(z12, GsVec(.8f, -3.5f, 0));
	SnManipulator* manip12 = e->get<SnManipulator>(11); // access one of the manipulators
	manip12->visible(false);
	lfoot = manip12->mat();
	lfoot = lfoot * lleg.inverse();
	
	GsModel *show13 = new GsModel;
	show13->load("..\\city\\ciity.obj");
	SnModel	*z13 = new SnModel(show13);
	add_model(z13, GsVec(160, -18.5f, 0));
	SnManipulator* manip13 = e->get<SnManipulator>(12); // access one of the manipulators
	manip13->visible(false);
	city = manip13->mat();
	manip13->initial_mat(manip13->mat()*scale);
	
	GsModel *show14 = new GsModel;
	show14->load("..\\city\\street.obj");
	SnModel	*z14 = new SnModel(show14);
	add_model(z14, GsVec(160, -18.5f, 0));
	show14->flat();
	SnManipulator* manip14 = e->get<SnManipulator>(13); // access one of the manipulators
	manip14->visible(false);
	street = manip14->mat();
	manip14->initial_mat(manip14->mat()*scale);

	GsModel *show15 = new GsModel;
	show15->load("..\\city\\wall.obj");
	SnModel	*z15 = new SnModel(show15);
	add_model(z15, GsVec(160, -18.5f, 0));
	show15->flat();
	SnManipulator* manip15 = e->get<SnManipulator>(14); // access one of the manipulators
	manip15->visible(false);
	manip15->initial_mat(manip15->mat()*scale);
	

	GsModel *show16 = new GsModel;
	show16->load("..\\IM\\Super Boo\\Obj\\S_Boo.obj");
	SnModel	*z16 = new SnModel(show16);
	add_model(z16, GsVec(300, 50, -40));
	show16->flat();
	SnManipulator* manip16 = e->get<SnManipulator>(15); // access one of the manipulators
	manip16->visible(false);
	Buu = manip16->mat();
	
	GsModel *show17 = new GsModel;
	show17->load("..\\Seahawk\\Seahawk.obj");
	SnModel	*z17 = new SnModel(show17);
	add_model(z17, GsVec(300, 80, 0));
	show17->flat();
	SnManipulator* manip17 = e->get<SnManipulator>(16); // access one of the manipulators
	manip17->visible(false);
	heli = manip17->mat();
	GsModel *show18 = new GsModel;
	show18->load("..\\Seahawk\\wing.obj");
	SnModel	*z18 = new SnModel(show18);
	add_model(z18, GsVec(300, 100, 20));
	show18->flat();
	SnManipulator* manip18 = e->get<SnManipulator>(17); // access one of the manipulators
	manip18->visible(false);
	wing = manip18->mat();
	wing = wing * heli.inverse();
	
	GsModel *show19 = new GsModel;
	show19->load("..\\red_car\\race_car.obj");
	SnModel	*z19 = new SnModel(show19);
	add_model(z19, GsVec(0, -5, 150));
	show19->flat();
	SnManipulator* manip19 = e->get<SnManipulator>(18); // access one of the manipulators
	manip19->visible(false);

	GsModel *show20 = new GsModel;
	show20->load("..\\red_car\\race_car.obj");
	SnModel	*z20 = new SnModel(show19);
	add_model(z20, GsVec(40, -5, -15));
	show20->flat();
	SnManipulator* manip20 = e->get<SnManipulator>(19); // access one of the manipulators
	manip20->visible(false);

	GsModel *show21 = new GsModel;
	show21->load("..\\red_car\\race_car.obj");
	SnModel	*z21 = new SnModel(show19);
	add_model(z21, GsVec(50, -5, -40));
	show21->flat();
	SnManipulator* manip21 = e->get<SnManipulator>(20); // access one of the manipulators
	manip21->visible(false);

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	_animating = true;
	circle();
	int ind = 0; // pick one child

	double frame_count = 0;
	double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
							  //double v = 4; // target velocity is 1 unit per second
	double t = 0, lt = 0, t0 = gs_time();
	do // run for a while:
	{
		while (t - lt<frdt) { ws_check(); t = gs_time() - t0; } // wait until it is time for next frame

		lt = t;
		 
		if (frame_count >= 0 && frame_count <= 10) {
			rotate(3, 1);
			rotate(6, -1);
			rotate(8, 1);
			rotate(7, -1);
			rotate(10, -1);
			rotate(9, -1);
			global(17, 1);
			rotate(14, -1);
			car_move(0);
			spin();
			spin2();
			circle();

		}
		if (frame_count > 10 && frame_count <= 20) {
			rotate(3, 1);
			rotate(6, -1);
			rotate(8, 1);
			rotate(7, -1);
			rotate(10, -1);
			rotate(9, -1);
			global(17, 1);
			rotate(14, -1);
			car_move(0);
			spin();
			spin2();
			circle();

		}
		if (frame_count > 20 && frame_count <= 30) {
			rotate(3, -1);
			rotate(6, 1);
			rotate(8, -1);
			rotate(7, 1);
			rotate(10, 1);
			rotate(9, 1);
			global (17, 1);
			rotate(14, -1);
			car_move(0);
			spin();
			spin2();
			circle();
		}
		if (frame_count > 30 && frame_count <= 40) {
			rotate(3, -1);
			rotate(6, 1);
			rotate(8, -1);
			rotate(7, 1);
			rotate(10, 1);
			rotate(9, 1);
			global(17, 1);
			rotate(14, -1);
			car_move(0);
			spin();
			spin2();
			circle();
		}
		if (frame_count > 40 && frame_count <= 50) {
			rotate(3, -1);
			rotate(6, 1);
			rotate(8, -1);
			rotate(7, 1);
			rotate(10, 1);
			rotate(9, 1);
			global(17, -1);
			rotate(14, -1);
			car_move(1);
			spin();
			spin2();
			circle();
		}
		if (frame_count > 50 && frame_count <= 60) {
			rotate(3, -1);
			rotate(6, 1);
			rotate(8, -1);
			rotate(7, 1);
			rotate(10, 1);
			rotate(9, 1);
			global(17, -1);
			rotate(14, -1);
			car_move(1);
			spin();
			spin2();
			circle();
		}
		if (frame_count > 60 && frame_count <= 70) {
			rotate(3, 1);
			rotate(6, -1);
			rotate(8, 1);
			rotate(7, -1);
			rotate(10, -1);
			rotate(9, -1);
			global(17, -1);
			rotate(14, -1);
			car_move(1);
			spin();
			spin2();
			circle();
		}
		if (frame_count > 70 && frame_count <= 80) {
			rotate(3, 1);
			rotate(6, -1);
			rotate(8, 1);
			rotate(7, -1);
			rotate(10, -1);
			rotate(9, -1);
			global(17, -1);
			rotate(14, -1);
			car_move(1);
			spin();
			spin2();
			circle();
		}
		if (frame_count == 81)
			//_animating = false;
			frame_count = 0;
		frame_count++;
		render(); // notify it needs redraw
		ws_check(); // redraw now
	} while (_animating);
}


void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard(const GsEvent &e)
{
	int ret = WsViewer::handle_keyboard(e); // 1st let system check events
	if (ret) return ret;

	switch (e.key)
	{
	case GsEvent::KeyEsc: gs_exit(); return 1;
	//case 'n': { bool b = !_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
	default: gsout << "Key pressed: " << e.key << gsnl;
	case 'w':global(12, 1); follow_view(0); //camera_view(); 
		render(); return 1;
	case 's':global(12, -1); follow_view(1); //camera_view(); 
		render(); return 1;
	case 'd':global(13, 1); follow_view(2); //camera_view(); 
		render(); return 1;
	case 'a':global(13, -1); follow_view(3); //camera_view(); 
		render(); return 1;
	
	case 'c':rotate(11, -1); render(); return 1;
	case 'v':rotate(11, 1); render(); return 1;
	case 'b':global(14, 1); render(); return 1;
	case 'm':circle(); render(); return 1;
	case GsEvent::KeyF1:moves(11, 1); render(); return 1;
	case GsEvent::KeyF2:moves(12, 1); render(); return 1;
	case GsEvent::KeyF3:moves(12, -1); render(); return 1;
	case GsEvent::KeyUp:global(12, 1); camera_view(0); //camera_view(); 
		render(); return 1;
	case GsEvent::KeyDown:global(12, -1); camera_view(1); //camera_view();
		render();return 1;
	case GsEvent::KeyRight:global(13, 1); camera_view(2); //camera_view();  
		render(); return 1;
	case GsEvent::KeyLeft:global(13, -1);camera_view(3); //camera_view(); 
		render(); return 1;
	case GsEvent::KeyBack:global(11, 1);  //camera_view(); 
		render(); return 1;
	case GsEvent::KeySpace:static_view();return 1;
	//case GsEvent::KeyDel:circle(); render(); return 1;


	return 0;
	}
}
int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}