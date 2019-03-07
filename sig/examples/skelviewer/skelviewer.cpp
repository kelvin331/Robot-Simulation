/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "skelviewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_dialogs.h>

# include <sigogl/ws_run.h>

void ManipPCB ( KnIkManipulator* ikm, void* udata )
{
	((MySkelViewer*)udata)->iksolved(ikm);
}

MySkelViewer::MySkelViewer ( KnSkeleton* sk, int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{ 
	// create my scene and viewer:
	_sksc = new KnScene; // KnScene derives SnGroup
	_sksc->connect ( sk ); // creates a scene for my skeleton

	// set visualization:
	bool viewskel=false, viewvisg=true, viewcolg=false, viewaxis=false;
	if ( sk->visgeos()==0 ) // if no geometries show the skeleton:
	{	viewskel=true; viewvisg=false; }
	_sksc->set_visibility ( viewskel/*skel*/, viewvisg/*visgeo*/, viewcolg/*colgeo*/, viewaxis/*axis*/ );

	// Extra settings:
	//sk->enforce_rot_limits ( false );

	rootg()->add ( _sksc );

	_sk=sk;
	_seljoint=0;

	UiPanel *p, *sp;
	p = uim()->add_panel ( 0, UiPanel::HorizLeft );

	_effs.sizecap(0,4);
	int s=sk->joints().size();
	p->add ( new UiButton ( "joints", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_jointsp=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiRadioButton ( sk->joints()[i]->name(), EvJoint+i, i==0? true:false ) );
			if ( sk->joints()[i]->ik() ) _effs.push()=sk->joints()[i];
		}
	}

	p->add ( new UiButton ( "values", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_valuesp=sp;
		for ( int i=0; i<6; i++ )
		{	_sl[i] = (UiSlider*)p->add ( new UiSlider ( "V:", EvP1+i ) );
			_sl[i]->all_events(true);
		}
		//if ( sk->joints().size() ) build_value_panel(0);
	}

	s=sk->postures().size();
	if ( s>0 )
	{	p->add ( new UiButton ( "postures", sp=new UiPanel(0,UiPanel::Vertical) ) );
		UiPanel* p=_posturesp=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiRadioButton ( sk->postures()[i]->name(), EvPosture+i, i==0? true:false ) );
		}
	}

	s=_effs.size();
	if ( s>0 )
	{	rootg()->add( _mg=new SnGroup );
		p->add ( new UiButton ( "effectors", sp=new UiPanel(0,UiPanel::Vertical) ) );
		UiPanel* p=_endeffs=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiButton ( _effs[i]->name(), EvEff+i ) );
			KnIkManipulator* ikm = new KnIkManipulator;
			ikm->init ( _effs[i]->ik() );
			ikm->solve_method ( KnIkManipulator::SearchOrbit, _sksc );
			ikm->post_callback ( ManipPCB, this );
			ikm->lines ( true );
			_mg->add ( ikm );
		}
	}

	p->add ( new UiButton ( "view", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_valuesp=sp;
		_vbut[0] = (UiCheckButton*)p->add ( new UiCheckButton ( "skeleton", EvView, viewskel ) );
		_vbut[1] = (UiCheckButton*)p->add ( new UiCheckButton ( "visgeo", EvView, viewvisg ) );
		_vbut[2] = (UiCheckButton*)p->add ( new UiCheckButton ( "colgeo", EvView, viewcolg ) );
		_vbut[3] = (UiCheckButton*)p->add ( new UiCheckButton ( "frames", EvView, viewaxis ) );
	}

	p->add ( new UiButton ( "info", EvInfo ) );
	p->add ( new UiButton ( "exit", EvExit ) );
}

void MySkelViewer::joint_info ( int jid )
{
	if ( jid<0 || jid>=_sk->joints().size() ) return; // protection
	KnJoint* j = _sk->joints()[jid];

	message().len(0); GsOutput o; o.init(message());
	o << "Name:[" << j->name() << "] ";
	o << "Parent:[" << (j->parent()?j->parent()->name():"null") << "] ";
	o << "Offset:[" << j->offset() << "] ";
	o << "PosDOFs:[" << (3-j->pos()->nfrozen()) << "] ";
	o << "Prepost:[" << (j->rot()->hasprepost()) << "] ";

	o << " Channels:[";
	const KnChannels& ch = _sk->channels();
	for ( int i=0, s=ch.size(); i<s; i++ )
	{	if ( j->name()==ch.cget(i).jname() )
		{	o << ch.cget(i).type_name();
			if ( i+1<s ) o<<gspc;
		}
	}
	o << "]";
}

void MySkelViewer::iksolved ( KnIkManipulator* ikm )
{
	message().set ( KnIk::message(ikm->result()) );
}

void MySkelViewer::build_value_panel ( int jid )
{
	KnJoint* j = _sk->joints()[jid];
	for ( int i=0; i<6; i++ ) _sl[i]->hide();

	if ( !j->pos()->frozen(0) )
	{	_sl[0]->activate();
	}
	if ( !j->pos()->frozen(1) )
	{	_sl[1]->activate();
	}
	if ( !j->pos()->frozen(2) )
	{	_sl[2]->activate();
	}

	if ( j->rot_type()==KnJoint::TypeQuat )
	{
	}
	else if ( j->rot_type()==KnJoint::TypeEuler )
	{	KnJointEuler* e = j->euler();
		const char* l[] = { "Rx", "Ry", "Rz" };
		for ( int d=0; d<3; d++ )
		{	if ( !e->frozen(d) )
			{	_sl[d+3]->label().set(l[d]);
				_sl[d+3]->activate();
				_sl[d+3]->range(e->lolim(d),e->uplim(d));
				_sl[d+3]->value(e->value(d));
			}
		}
	}
	else if ( j->rot_type()==KnJoint::TypeST )
	{	KnJointST* st = j->st();
		_sl[3]->label().set("Sx:");	_sl[3]->activate();	_sl[3]->range(-gspi,gspi); _sl[3]->value(st->swingx());
		_sl[4]->label().set("Sy:");	_sl[4]->activate();	_sl[4]->range(-gspi,gspi); _sl[4]->value(st->swingy());
		if ( !st->twist_frozen() )
		{	_sl[5]->label().set("Tw:");
			_sl[5]->activate();
			_sl[5]->range(st->twist_lolim(),st->twist_uplim());
			_sl[5]->value(st->twist());
		}
	}

	UiPanel* p=_valuesp;
	p->changed ( UiElement::NeedsRebuild );
//	p->rebuild();
}

void MySkelViewer::update_joint ( int si, int jid )
{
	KnJoint* j = _sk->joints()[_seljoint];
	if ( si<3 ) // position DOFs
	{ // to be completed
	}
	else // rotation DOFs
	{	if ( j->rot_type()==KnJoint::TypeEuler )
		{	if ( si==3 ) j->euler()->value ( 0, _sl[3]->value() );
			else if ( si==4 ) j->euler()->value ( 1, _sl[4]->value() );
			else if ( si==5 ) j->euler()->value ( 2, _sl[5]->value() );
		}
		if ( j->rot_type()==KnJoint::TypeST )
		{	if ( si<5 )
				j->st()->swing ( _sl[3]->value(), _sl[4]->value() );
			else
				j->st()->twist ( _sl[5]->value() );
		}
		else
		{ // to be completed with xyz -> quat
		}
	}
}

int MySkelViewer::uievent ( int e )
{
	if ( e>=EvP1 && e<=EvR3 )
	{	update_joint(e-EvP1,_seljoint);
		_sksc->update();
		render();
	}
	else if ( e>=EvPosture && e<EvPosture+100 )
	{	KnPosture* p = _sk->postures()[e-EvPosture];
		p->apply();
		_sksc->update();
		message()=""; GsOutput o; o.init(message());
		p->output ( o, false, true );
		return 1;
	}
	else if ( e>=EvEff && e<EvEff+100 )
	{	KnJoint* j = _effs[e-EvEff];
		_mg->get<KnIkManipulator>(e-EvEff)->lines( j->ik()->lines()? false:true );
		//_sksc->update();
		render ();
		return 1;
	}
	else if ( e>=EvJoint && e<EvJoint+100 )
	{	_seljoint = e-EvJoint;
		joint_info ( _seljoint );
		build_value_panel ( _seljoint );
		return 1;
	}
	else switch ( e )
	{	//case EvInfo: info(); return 1;

		case EvView:
		_sksc->set_visibility ( _vbut[0]->value(), _vbut[1]->value(), _vbut[2]->value(), _vbut[3]->value() );
		return 1;

		case EvExit: ws_exit(); return 1;
	}

	return WsViewer::uievent(e);
}
