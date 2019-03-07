/*=======================================================================
   Copyright (c) 2018 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

//================================= ms windows port ==========================================

# include <sig/gs.h>

# ifdef GS_WINDOWS

# include <sig/gs_array.h>
# include <sig/gs_buffer.h>
# include <sig/gs_output.h>
# include <sigogl/ws_window.h>
# include <sigogl/gl_loader.h>
# include <sigogl/gl_resources.h>
# include <sigogl/ws_osinterface.h>
# include <stdlib.h>

# include <sig/gs_string.h>
# include <Shlobj.h>
# include <Commdlg.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1 // trace main functions
//# define GS_USE_TRACE2 // trace more functions
//# define GS_USE_TRACE5 // events
//# define GS_USE_TRACE6 // more events
//# define GS_USE_TRACE7 // mouse move event
# include <sig/gs_trace.h>

# define WIN32_LEAN_AND_MEAN 1 // exclude rarely-used stuff from Windows headers
# include <windows.h>

//===== Global Data ===========================================================================
struct SwSysWin; // fwd decl
static GsBuffer<SwSysWin*> AppWindows;
static HINSTANCE	AppInstance=0;
static HINSTANCE	AppPrevInstance=0;
static MSG			AppMsg;
static char*		AppName="SIGAppClass";
static gsint16		AppNumVisWindows=0;
static gsint16		DialogRunning=0;
static int (*AppCallBack)( const GsEvent& ev, void* wdata )=0;

//===== Declarations ==========================================================================
static LRESULT CALLBACK WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

//===== Window Data ===========================================================================
struct SwSysWin
{	char*	 label;			// window label
	WsWindow* uwindow;		// user window data
	GsEvent  event;			// event being sent to window
	gscbool  justresized;	// flag to allow client area correction
	gscbool  needsinit;		// flag to call client init funtion before first paint
	gscbool  visible;		// visible state according to calls to show and hide
	gscbool	 isdialog;		// dialogs need special treatment
	gscbool  redrawcalled;	// dialogs need special treatment
	HWND	 window;		// window handle
	HDC		 gldevcontext;	// opengl device context
	HGLRC	 glrendcontext;	// opengl rendering context
	SwSysWin() { label=0; uwindow=0; }
   ~SwSysWin(); // destructor
};

SwSysWin::~SwSysWin()
{
	GS_TRACE1 ( "SwSysWin Destructor ["<<label<<']' );
	delete uwindow; // destructor will call wsi_del_win()
	free ( label );
	if ( isdialog ) DialogRunning--;
	for ( int i=0; i<AppWindows.size(); i++ )
	{	if ( AppWindows[i]==this ) { AppWindows.remove(i); break; } }
}

//===== Functions =============================================================================

void wsi_del_win ( void *win ) // this function is called by the WsWindow destructor, possibly by the user
{
	GS_TRACE1 ( "wsi_del_win ["<<((SwSysWin*)win)->label<<"]..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( sw->isdialog ) { DialogRunning--; sw->isdialog=0; } // important to allow other windows to receive events
	wsi_win_hide ( sw );

	if ( sw->glrendcontext && sw->gldevcontext ) // the user called this function
	{	sw->uwindow=0; // if user deletes sw, this is being called before SwSysWin destructor, stop a 2nd delete
		PostMessage ( sw->window, WM_DESTROY, 0, 0 ); // sw will then be deleted from WndProc
	}
	else // this call came from SwSysWin desctructor triggered by a DESTROY event
	{	// nothing more to do
	}
}

void* wsi_new_win ( int x, int y, int w, int h, const char* label, WsWindow* uwindow, int mode )
{
	GS_TRACE1 ( "wsi_new_win ["<<label<<"]..." );

	GlResources::load_configuration_file (); // only overall 1st call will actually load config file

	SwSysWin* sw = new SwSysWin;
	sw->label = _strdup ( label );
	sw->uwindow = uwindow;
	sw->justresized = 1;
	sw->needsinit = 1;
	sw->event.width = w;
	sw->event.height = h;
	sw->visible = 0;
	sw->isdialog = mode;
	sw->redrawcalled = 0;
	sw->window = 0;

	if ( mode>0 ) DialogRunning++;

	WNDCLASSEX wc;
	ATOM atom = GetClassInfoEx ( AppInstance, AppName, &wc );

	if ( atom==0 )
	{	GS_TRACE1 ( "Registering window..." );
		wc.cbSize		 = sizeof(WNDCLASSEX);
		wc.style		 = 0;
		wc.lpfnWndProc   = WndProc;
		wc.cbClsExtra	 = 0;
		wc.cbWndExtra	 = 0;
		wc.hInstance	 = AppInstance;
		wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor	     = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = AppName;
		wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);
		if ( !RegisterClassEx(&wc) )
		{	MessageBox ( NULL, "Window Registration Failed !", "", MB_ICONEXCLAMATION|MB_OK ); exit(0); }
	}

	// the following corrections will not always work and so we will re-check at the window creation event
	int dw = 2*(GetSystemMetrics(SM_CXSIZEFRAME)+GetSystemMetrics(SM_CXEDGE));
	int dh = 2*(2*(GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYEDGE))+GetSystemMetrics(SM_CYBORDER));

	// update default sizes:
	const int minw=50, minh=50; // it seems windows may not always accept sizes smaller than about 100
	int scw, sch; 
	wsi_screen_resolution ( scw, sch );
	if ( w<minw ) w=minw;
	if ( h<minh ) h=minh;
	if ( x<0 ) x = (scw-w)/2;
	if ( y<0 ) y = (sch-h)/2;

	// dialog boxes are simulated with mode>0 (DialogBox() function does not return until callback function terminates)
	GS_TRACE1 ( "Creating window..." );
	sw->window = CreateWindowEx (
		mode>0? WS_EX_DLGMODALFRAME|WS_EX_TOPMOST : WS_EX_ACCEPTFILES, // recall WS_EX_CLIENTEDGE WS_EX_TOOLWINDOW
		AppName, 
		sw->label,
		mode==2? WS_SIZEBOX : mode==1? WS_DLGFRAME : WS_OVERLAPPEDWINDOW,
		x<=0? CW_USEDEFAULT:x,
		y<=0? CW_USEDEFAULT:y,
		w<=0? CW_USEDEFAULT:w+dw,
		h<=0? CW_USEDEFAULT:h+dh,
		NULL, // parent: perhaps should use first non-dialog active window
		NULL, // menu
		AppInstance, 
		sw );

	if ( sw->window==NULL)
	{	MessageBox ( NULL, "Window Creation Failed !", "", MB_ICONEXCLAMATION | MB_OK );
		exit(0);
	}

	// The OpenGL context configuration is by now fixed. 
	// If it becomes customizable, it should be the same for all windows
	// otherwise the sharing of contexts among all windows will not work.
	int buffers = 2;
	BYTE zbufferbits = 32;
   
	// set buffers:
	DWORD flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	if ( buffers==2 ) flags |= PFD_DOUBLEBUFFER; // set to be double buffered 

	// create an OpenGL rendering context:
	sw->gldevcontext = GetDC ( sw->window );
   
	PIXELFORMATDESCRIPTOR pfd = 
	{	sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd 
		1,				// version number, should be 1
		flags,			// properties of the pixel buffer
		PFD_TYPE_RGBA,	// RGBA type 
		24,				// 24-bit color depth 
		8, 0,			// red bits with 0 shift value
		8, 0,			// green bits with 0 shift value
		8, 0,			// blue bits with 0 shift value
		8, 0,			// alpha bits with 0 shift value
		0,				// no accumulation buffer 
		0, 0, 0, 0,		// accum bits ignored 
		zbufferbits,	// 32-bit z-buffer	 
		0,				// no stencil buffer 
		0,				// no auxiliary buffer 
		PFD_MAIN_PLANE,	// main layer 
		0,				// reserved
		0, 0, 0			// layer masks ignored 
	};

	int pixformat = ChoosePixelFormat ( sw->gldevcontext, &pfd );
	if ( pixformat==NULL)
	{	MessageBox ( NULL, "Pixel Format Error !", "", MB_ICONEXCLAMATION | MB_OK );
		exit(0);
	}
	   
	if ( !SetPixelFormat ( sw->gldevcontext, pixformat, &pfd ) )
	{	MessageBox ( NULL, "Pixel Format Set Error !", "", MB_ICONEXCLAMATION | MB_OK );
		exit(0);
	}

	sw->glrendcontext = wglCreateContext ( sw->gldevcontext );
	if ( AppWindows.size() ) wglShareLists ( AppWindows[0]->glrendcontext, sw->glrendcontext );

	AppWindows.push ( sw );

	if ( sw->glrendcontext==NULL)
	{	MessageBox ( NULL, "Could Not Create OpenGL Context !", "", MB_ICONEXCLAMATION | MB_OK );
		exit(0);
	}

	SetFocus ( sw->window ); // needed if this window is not the first one
	return (void*)sw;
}

const char* wsi_win_label ( void* win )
{
	GS_TRACE2 ( "wsi_win_label ["<<((SwSysWin*)win)->label<<"] label requested." );
	return ((SwSysWin*)win)->label;
}

void wsi_win_label ( void* win, const char* label )
{
	GS_TRACE2 ( "wsi_win_label ["<<((SwSysWin*)win)->label<<"] to "<<label<<"..." );
	SwSysWin* sw = (SwSysWin*)win;
	free ( sw->label );
	sw->label = _strdup ( label );
	SetWindowText ( sw->window, sw->label );
}

void wsi_win_show ( void* win )
{
	GS_TRACE2 ( "wsi_win_show..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( !sw->visible ) { AppNumVisWindows++; sw->visible=1; }
	ShowWindow ( sw->window, 1 );
}

void wsi_win_hide ( void* win )
{
	GS_TRACE2 ( "wsi_win_hide..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( sw->visible ) { AppNumVisWindows--; sw->visible=0; }
	ShowWindow ( sw->window, 0 );
}

void wsi_win_move ( void* win, int x, int y, int w, int h )
{
	GS_TRACE2 ( "wsi_win_move..." );
	SwSysWin* sw = (SwSysWin*)win;
	sw->justresized = true;
	RECT rc; GetClientRect ( sw->window, &rc );
	RECT rw; GetWindowRect ( sw->window, &rw );
	int dw = (rw.right-rw.left)-rc.right;
	int dh = (rw.bottom-rw.top)-rc.bottom;
	if ( x<0 ) x = rw.left;
	if ( y<0 ) y = rw.top;
	if ( w<0 ) w = sw->event.width; else sw->event.width = w;
	if ( h<0 ) h = sw->event.height; else sw->event.height = h;
	MoveWindow ( sw->window, x, y, w+dw, h+dh, TRUE );
}

void wsi_win_pos ( void* win, int& x, int& y )
{
	GS_TRACE2 ( "wsi_win_pos..." );
	SwSysWin* sw = (SwSysWin*)win;
	RECT r; 
	GetWindowRect ( sw->window, &r );
	x=r.left; y=r.top;
}

void wsi_win_size ( void* win, int& w, int& h )
{
	GS_TRACE2 ( "wsi_win_size..." );
	SwSysWin* sw = (SwSysWin*)win;
	RECT r;
	GetClientRect ( sw->window, &r );
	w=r.right; h=r.bottom;
}

void wsi_win_redraw ( void* win )
{
	GS_TRACE2 ( "wsi_win_redraw..." );
	SwSysWin* sw = (SwSysWin*)win;
	if ( sw->redrawcalled ) return; // already invalidated
	InvalidateRect ( sw->window, NULL, TRUE );
	sw->redrawcalled = 1;
	// Recall this function if drawing before emptying events is needed at some point:
	// UpdateWindow (sw->window); // sends a WM_PAINT msg bypassing the application queue (if update region not empty)
}

void wsi_win_setoglcontext ( void* win, bool b )
{
	if ( b )
		wglMakeCurrent ( ((SwSysWin*)win)->gldevcontext, ((SwSysWin*)win)->glrendcontext );
	else
		wglMakeCurrent ( NULL, NULL );
}

bool wsi_win_visible ( void* win )
{
	GS_TRACE2 ( "wsi_win_visible..." );
	return IsWindowVisible ( ((SwSysWin*)win)->window )==1;
}

bool wsi_win_minimized ( void* win )
{
	GS_TRACE2 ( "wsi_win_iconized..." );
	return IsIconic ( ((SwSysWin*)win)->window )==1;
}

int wsi_num_windows ()
{
	GS_TRACE2 ( "wsi_num_windows..." );
	return AppNumVisWindows;
}

void wsi_activate_ogl_context ( void* win )
{
	GS_TRACE2 ( "wsi_set_ogl_context..." );
	wglMakeCurrent ( ((SwSysWin*)win)->gldevcontext, ((SwSysWin*)win)->glrendcontext );
}

void* wsi_get_ogl_procedure ( const char *name )
{
	void *p = (void*)wglGetProcAddress(name);
	if ( !p || (long(p)>=-1 && long(p)<=0x3) ) // cover failure values that may be returned
	{	HMODULE module = LoadLibraryA("opengl32.dll"); // will not load multiple times but increment reference count of handle
		p = !module? 0: (void*)GetProcAddress(module, name);
	}
	return p;
}

int wsi_check ()
{
	while ( PeekMessage(&AppMsg, 0, 0, 0, PM_REMOVE) )
	{	TranslateMessage (&AppMsg);
		DispatchMessage (&AppMsg);
	}
	return AppNumVisWindows;
}

//this function is not needed:
// checks if there are window events to be processed; will return 0 or 1
//int wsi_peek ()
//{
//	return PeekMessage ( &AppMsg, 0, 0, 0, PM_NOREMOVE );
//}

void wsi_screen_resolution ( int& w, int& h )
{
	w = GetSystemMetrics ( SM_CXSCREEN );
	h = GetSystemMetrics ( SM_CYSCREEN );
}

//==== WndProc ==============================================================================

static void setkeycode ( GsEvent& e, WPARAM wParam )
{
	const gsbyte NS[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
	if ( (wParam>='0' && wParam<='9') )
	{ e.key=wParam; e.character=(gsbyte)(e.shift?NS[wParam-'0']:wParam); return; }

	if ( (wParam>='A' && wParam<='Z') )
	{ e.key=wParam-'A'+'a'; e.character=(gsbyte)(e.shift?wParam:e.key); return; }

	e.character = 0;
	# define RET(x) e.key=GsEvent::x; return
	# define RET1(x) e.key=x; e.character=(gsbyte)x; return
	# define RET2(x,y) e.key=x; e.character=(gsbyte)(e.shift?y:x); return
	switch (wParam)
	{	case VK_LEFT	: RET(KeyLeft); case VK_RIGHT	: RET(KeyRight);
		case VK_UP		: RET(KeyUp);	case VK_DOWN	: RET(KeyDown);
		case VK_ESCAPE	: RET(KeyEsc);	case VK_SPACE	: RET1(' ');
		case VK_BACK	: RET(KeyBack);
		case VK_HOME	: RET(KeyHome); case VK_END		: RET(KeyEnd);
		case VK_PRIOR	: RET(KeyPgUp); case VK_NEXT	: RET(KeyPgDn);
		case VK_TAB		: RET1('\t');	case VK_RETURN	: RET(KeyEnter);
		case VK_INSERT	: RET(KeyIns);	case VK_DELETE	: RET(KeyDel);
		case VK_DIVIDE	: RET1('/');	case VK_MULTIPLY: RET1('*');
		case VK_ADD		: RET1('+');	case VK_SUBTRACT: RET1('-');
		case 192: RET2('`','~'); case 189: RET2('-','_');  case 187: RET2('=','+'); 
		case 219: RET2('[','{'); case 221: RET2(']','}');  case 220: RET2('\\','|'); 
		case 186: RET2(';',':'); case 222: RET2('\'','"'); case 188: RET2(',','<'); 
		case 190: RET2('.','>'); case 191: RET2('/','?');
	}
	if ( wParam>=VK_F1 && wParam<=VK_F12 ) e.key=GsEvent::KeyF1+(wParam-VK_F1);
}

static void setstate ( GsEvent& e )
{
	e.shift = GetKeyState(VK_SHIFT)&128? 1:0;
	e.ctrl  = GetKeyState(VK_CONTROL)&128? 1:0;
	e.alt   = GetKeyState(VK_MENU)&128? 1:0;
	e.button1 = GetKeyState(VK_LBUTTON)&128? 1:0;
	e.button2 = GetKeyState(VK_MBUTTON)&128? 1:0;
	e.button3 = GetKeyState(VK_RBUTTON)&128? 1:0;
}

// this makes sure the client size is the specified one (and not the window size)
static void fixsize ( SwSysWin* sw )
{
	RECT c;
	GetClientRect ( sw->window, &c );
	if ( c.right==sw->event.width && c.bottom==sw->event.height ) return;
	GS_TRACE1 ( "FIXING SIZE..." );
	RECT w;
	GetWindowRect ( sw->window, &w );
	GS_TRACE1 ( "Client: "<<c.right<<"x"<<c.bottom );
	GS_TRACE1 ( "Window: "<<(w.bottom-w.top-1)<<"x"<<(w.right-w.left-1) );
	int dw = (w.right-w.left)-c.right;
	int dh = (w.bottom-w.top)-c.bottom;
	int nw = sw->event.width + dw;
	int nh = sw->event.height + dh;
	MoveWindow ( sw->window, w.left, w.top, nw, nh, TRUE );
}

// The following are inline friend functions of WsWindow:
inline void sysdraw ( WsWindow* win ) { win->draw(win->_glrenderer); }
inline void sysinit ( WsWindow* win, int w, int h ) { win->init(win->_glcontext,w,h); }
inline void sysresize ( WsWindow* win, int w, int h ) { win->resize(win->_glcontext,w,h); }
inline void sysevent ( WsWindow* win, GsEvent& e ) { win->handle(e); }

static LRESULT CALLBACK WndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg==WM_NCCREATE ) // associate SwSysWin data to the window (WM_NCCREATE happens before WM_CREATE)
	{	GS_TRACE5 ( "WM_NCCREATE..." );
		LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		void* lpCreateParam = create_struct->lpCreateParams;
		SwSysWin* sw = reinterpret_cast<SwSysWin*>(lpCreateParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(sw));
		return DefWindowProc ( hWnd, uMsg, wParam, lParam );
	}

	SwSysWin* sw = reinterpret_cast<SwSysWin*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (!sw) { return DefWindowProc ( hWnd, uMsg, wParam, lParam ); }

	if ( uMsg==WM_PAINT ) // process paint first 
	{	GS_TRACE5 ( "WM_PAINT ["<<sw->label<<"]..." );
		//static int i=0; gsout<<"WM_PAINT ["<<(i++)<<"]...\n";
		DefWindowProc ( hWnd, uMsg, wParam, lParam ); // Windows has also to process it
		wglMakeCurrent ( sw->gldevcontext, sw->glrendcontext ); // needed when working with multiple windows
		if (sw->needsinit)
		{	gl_load_and_initialize (); // only overall 1st call will actually load ogl
			int w, h; 
			wsi_win_size ( sw, w, h ); 
			sysinit ( sw->uwindow, w, h );
			sw->needsinit=0;
		}
		sysdraw ( sw->uwindow ); // this will call user's draw function
		SwapBuffers ( sw->gldevcontext );
		wglMakeCurrent ( NULL, NULL ); // needed when working with multiple windows
		sw->redrawcalled=0;
		return 0;
	}

	if ( DialogRunning && !sw->isdialog ) return DefWindowProc ( hWnd, uMsg, wParam, lParam );

	GsEvent& e = sw->event;
	e.type = GsEvent::None;
	e.wheelclicks = 0; 
	e.button = 0;
	e.key = 0;

	switch(uMsg) // process other events
	{
		case WM_KEYDOWN: // (we do not read WM_CHAR events)
		case WM_SYSKEYDOWN:
			GS_TRACE5 ( "WM_KEYDOWN..." );
			setstate ( e );
			setkeycode ( e, wParam );
			if ( e.key==0 ) return DefWindowProc ( hWnd, uMsg, wParam, lParam );
			e.type = GsEvent::Keyboard;
			break;

		case WM_KEYUP  :
		case WM_SYSKEYUP:
			setstate ( e ); // update state in event
			return DefWindowProc ( hWnd, uMsg, wParam, lParam ); // not currently in use
			break;

		// we call setstate below to have correct events in all cases with multiple windows
		# define SETLMOUSE e.lmousex=e.mousex; e.lmousey=e.mousey; e.mousex=LOWORD(lParam); e.mousey=HIWORD(lParam);
		# define MOUSEEV(t,i,but,bs) e.type=t; e.button=i; e.but=bs; SETLMOUSE; setstate(e)
		case WM_LBUTTONDOWN :
			GS_TRACE5 ( "WM_LBUTTONDOWN..." );
			MOUSEEV ( GsEvent::Push, 1, button1, 1 );
			break;
		case WM_MBUTTONDOWN :
			GS_TRACE5 ( "WM_MBUTTONDOWN..." );
			MOUSEEV ( GsEvent::Push, 2, button2, 1 );
			break;
		case WM_RBUTTONDOWN :
			GS_TRACE5 ( "WM_RBUTTONDOWN..." );
			MOUSEEV ( GsEvent::Push, 3, button3, 1 );
			break;
		case WM_LBUTTONUP :
			GS_TRACE5 ( "WM_LBUTTONUP..." );
			MOUSEEV ( GsEvent::Release, 1, button1, 0 );
			break;
		case WM_MBUTTONUP :
			GS_TRACE5 ( "WM_MBUTTONUP..." );
			MOUSEEV ( GsEvent::Release, 2, button2, 0 );
			break;
		case WM_RBUTTONUP :
			GS_TRACE5 ( "WM_RBUTTONUP..." );
			MOUSEEV ( GsEvent::Release, 3, button3, 0 );
			break;
		case WM_MOUSEMOVE :
			GS_TRACE7 ( "WM_MOUSEMOVE..." );
			setstate(e);
			e.type = e.button1||e.button3? GsEvent::Drag : GsEvent::Move;
			SETLMOUSE;
			if ( GetFocus()!=hWnd ) SetFocus ( hWnd );
			break;
		# undef MOUSEEV
		# undef SETLMOUSE

		case WM_MOUSEWHEEL:
			GS_TRACE5 ( "WM_MOUSEWHEEL..." );
			e.type = GsEvent::Wheel;
			e.wheelclicks = GET_WHEEL_DELTA_WPARAM(wParam);
			break;

		case WM_SIZE :
			GS_TRACE5 ( "WM_SIZE "<<sw->event.width<<"x"<<sw->event.height<<" to "<<LOWORD(lParam)<<"x"<<HIWORD(lParam)<<"..." );
			if (sw->justresized) 
			{	sw->justresized=false; fixsize(sw); 
				RECT r; GetClientRect ( sw->window, &r ); // get client rect in case size was adjusted
				e.width = r.right; // if not adjusted could use LOWORD(lParam)
				e.height = r.bottom; // if not adjusted could use HIWORD(lParam)
			}
			else
			{	e.width = LOWORD(lParam);
				e.height = HIWORD(lParam);
			}
			if ( e.width==0 || e.height==0 ) return 0; // window is being minimized
			// Only signal user's window if the window and OpenGL are initialized:
			if ( !sw->needsinit ) 
			{	wglMakeCurrent ( sw->gldevcontext, sw->glrendcontext );
				sysresize ( sw->uwindow, e.width, e.height );
				wglMakeCurrent ( NULL, NULL );
			}
			InvalidateRect ( sw->window, NULL, TRUE ); // force paint since a shrink will not do it
			return 0;

		case WM_DESTROY :
			GS_TRACE5 ( "WM_DESTROY ["<<sw->label<<"]..." );
			PostQuitMessage ( 0 );
			if ( AppWindows.size()>1 ) // Do not delete last OpenGL Context because all shared resources would be lost
			{	wglDeleteContext ( sw->glrendcontext ); sw->glrendcontext=NULL; }
			ReleaseDC ( hWnd, sw->gldevcontext ); sw->gldevcontext=NULL;
			delete sw;
			return 0;

		case WM_SYSCOMMAND:
			switch ( wParam & 0xFFF0 ) // system commands that we want to ignore are detected here
			{	case SC_KEYMENU: case SC_SCREENSAVE: return 0; // ignore
				default: return DefWindowProc ( hWnd, uMsg, wParam, lParam ); // process the others
			}
			break;

		default: 
			#ifdef GS_USE_TRACE6
			if ( uMsg==WM_NCMOUSELEAVE ) gsout<<"WM_NCMOUSELEAVE\n";
			gsout.putf("Event not handled: %0x\n",uMsg); 
			#endif
			return DefWindowProc ( hWnd, uMsg, wParam, lParam );
	}

	sysevent ( sw->uwindow, e );
	return 0;
}

//=============================== native file dialogs ==========================================

# define FILEBUFSIZE 256
static GsString FileBuf;

// filter format:  "*.txt;*.log"
static const char* filedlg ( bool open, const char* msg, const char* file, const char* filter, GsArray<const char*>* multif )
{
	int bufsize = multif? 4*FILEBUFSIZE:FILEBUFSIZE;
	FileBuf.reserve ( bufsize );
	FileBuf.len(0);

	GsString path=file;
	if ( path.len() )
	 { extract_filename ( path, FileBuf );
	   path.replall ( '/', '\\' ); // put in windows format
	   if ( path.lchar()=='\\' ) path.lchar(0);
	   if ( !gs_exists(path) ) path.len(0);
	 }
	//gsout<<"path:"<<path<<gsnl;
	//gsout<<"file:"<<FileBuf<<gsnl;

	GsString filterdesc;
	if ( filter&&filter[0] ) filterdesc<<"Filtered Files ("<<filter<<")|"<<filter<<'|';
	filterdesc.append ( "All Files|*.*||" );
	//gsout<<"filter:"<<filterdesc<<gsnl;
	filterdesc.replall ( '|', '\0' );
 	//HRESULT lHResult = CoInitializeEx(NULL,0); // not sure if this is needed

	OPENFILENAME ofn;
	memset ( &ofn, 0, sizeof(OPENFILENAME) ); // make sure everything is set to 0
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= AppWindows.size()? AppWindows.top()->window:0;
	ofn.hInstance		= 0;
	ofn.lpstrFilter		= filterdesc.pt();
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter	= 0;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= &FileBuf[0];
	ofn.nMaxFile		= bufsize; // size in characters allocated in lpstrFile, FALSE returned if buffer too small
	ofn.lpstrFileTitle	= NULL; // file name and extension (without path information) of the selected file. 
	ofn.nMaxFileTitle	= 0; // size of buffer in previous item
	ofn.lpstrInitialDir	= path.len()? path.pt():NULL;
	ofn.lpstrTitle		= msg;
	if (open) ofn.Flags	= OFN_EXPLORER|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;
		 else ofn.Flags	= OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR; // OFN_NOCHANGEDIR needed: restores app current dir
	ofn.nFileOffset		= 0;
	ofn.nFileExtension	= 0;
	ofn.lpstrDefExt		= NULL;
	ofn.lCustData		= 0L;
	ofn.lpfnHook		= NULL;
	ofn.lpTemplateName	= NULL;

	const char* ret=0;

	if ( open )
	{	if ( multif ) { ofn.Flags|=OFN_ALLOWMULTISELECT; multif->size(0); }

		// There is always an extra NULL character after the last file name.
		// If multiple files were selected the directory and file names are NULL separated with an extra NULL at the end
		if ( !GetOpenFileName(&ofn) )
		{	FileBuf.clear(); }
		else
		{	if ( !multif )
			{	FileBuf.calclen(); FileBuf.compress(); ret=&FileBuf[0]; }
			else
			{	ret = &FileBuf[0];
				const char* pt = &ret[strlen(ret)+1];
				if ( *pt ) // multiple files were selected
				{ while ( *pt ) { multif->push()=pt; while (*pt) pt++; pt++; } }
			}
		}
	}
	else // save file mode
	{	if ( GetSaveFileName(&ofn) )
		{	FileBuf.calclen(); FileBuf.compress(); ret=&FileBuf[0]; }
		else
		{	FileBuf.clear(); }
	}

	// if (lHResult==S_OK || lHResult==S_FALSE) CoUninitialize(); // not sure if this is needed

	return ret;
 }

const char* wsi_open_file_dialog ( const char* msg, const char* file, const char* filter, GsArray<const char*>* multif )
{
	return filedlg ( true, msg, file, filter, multif );
}

const char* wsi_save_file_dialog ( const char* msg, const char* file, const char* filter )
{
	return filedlg ( false, msg, file, filter, 0 );
}

static int CALLBACK BrowseCallbackProc ( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{	case BFFM_INITIALIZED: if(lpData) SendMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData); break;
	}
	return 0;
}

const char* wsi_select_folder ( const char* msg, const char* folder )
{
	//HRESULT lHResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	FileBuf.len ( MAX_PATH );
	if (folder) FileBuf=folder;	else GetCurrentDirectory(MAX_PATH,&FileBuf[0]);
	BROWSEINFOA bInfo;
	bInfo.hwndOwner			= AppWindows.size()? AppWindows.top()->window:0;
	bInfo.pidlRoot			= NULL;
	bInfo.pszDisplayName	= &FileBuf[0]; // will return folder name without full path
	bInfo.lpszTitle			= msg;
	bInfo.ulFlags			= BIF_USENEWUI|BIF_NEWDIALOGSTYLE;
	bInfo.lpfn				= BrowseCallbackProc;
	bInfo.lParam			= (long)&FileBuf[0];
	bInfo.iImage			= 0;

	LPITEMIDLIST lpItem = SHBrowseForFolderA ( &bInfo );
	if ( lpItem )
	{	SHGetPathFromIDListA ( lpItem , &FileBuf[0] );
		return FileBuf.pt();
	}
	else
	{	FileBuf.clear();
		return 0;
	}
	//if ( lHResult==S_OK || lHResult==S_FALSE ) CoUninitialize();
}

//=============================== misc windows functions ==========================================

// Main Function Arguments:
static GsBuffer<char*> AppArgs;

char** wsi_program_argv ()
{
	return &AppArgs[0];
}

int wsi_program_argc ()
{
	return AppArgs.size()-1;
}

// the main function entry point must be provided elsewhere:
extern int main ( int, char** );

int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	GS_TRACE1 ( "WinMain..." );
	AppInstance = hInstance;
	AppPrevInstance = hPrevInstance;

	// Converting command line arguments to argv format:
	char* pt = GetCommandLine();
	if ( *pt=='"' ) pt++; // executable name may come between quotes, remove first one here
	while ( true )
	{	AppArgs.push(pt);
		while ( *pt && *pt!=' ' ) pt++; // skip argument
		if ( AppArgs.size()==1 && *(pt-1)=='"' ) *(pt-1)=0; // remove end quote in executable name
		if ( *pt==0 ) break;
		*pt++ = 0; // mark end of argument
		while ( *pt && *pt==' ' ) pt++; // skip any extra spaces
		if ( *pt==0 ) break;
	}
	AppArgs.push(0); // for compatibility last argv should be null

	// call the user-provided main:
	return main ( AppArgs.size()-1, &AppArgs[0] );

	// Reminder: SIG config file loading and OpenGL initialization are performed
	// automatically at the time of opening the first graphical window.
}

# endif // GS_WINDOWS

//================================ End of File =================================================
