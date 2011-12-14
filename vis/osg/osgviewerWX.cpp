#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

// For compilers that support precompilation, includes "wx.h".
//#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// For wxCURSOR_BLANK below, but isn't used a.t.m.
//#ifdef WIN32
//#include "wx/msw/wx.rc"
//#endif

#include "osgviewerWX.h"

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <wx/image.h>
#include <wx/menu.h>

using namespace std;

const int REFRESH_INTERVAL = 16;  // ms

enum {
	REFRESH_TIMER_ID = 1,
};

void read_nonblock(string &str) {
	vector<char> buf(1024);
	int n = fread(&buf[0], sizeof(char), 1023, stdin);
	buf[n] = '\0';
	str += &buf[0];
}

bool wxOsgApp::OnInit() {
	int width = 800;
	int height = 600;

	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

	MainFrame *frame = new MainFrame(wxSize(width, height));

	/* Show the frame */
	frame->Show(true);
	return true;
}

IMPLEMENT_APP(wxOsgApp)

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_TIMER(REFRESH_TIMER_ID, MainFrame::on_refresh)
END_EVENT_TABLE()

MainFrame::MainFrame(const wxSize &size)
: wxFrame(NULL, wxID_ANY, wxT("Viewer"), wxDefaultPosition, size),
  refresh_timer(this, REFRESH_TIMER_ID)
{
	vector<int> attributes = { int(WX_GL_DOUBLEBUFFER), WX_GL_RGBA, WX_GL_DEPTH_SIZE, 8, WX_GL_STENCIL_SIZE, 8, 0 };
	canvas = new OSGCanvas(this, wxID_ANY, wxDefaultPosition, size, wxSUNKEN_BORDER, wxT("osgviewerWX"), &attributes[0]);
	
	canvas_sizer = new wxBoxSizer(wxVERTICAL);
	btn_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	canvas_sizer->Add(btn_sizer, 0, wxEXPAND, 0);
	canvas_sizer->Add(canvas, 1, wxEXPAND, 0);
	SetSizer(canvas_sizer);

	viewer = new osgViewer::Viewer;
	viewer->getCamera()->setGraphicsContext(canvas->GetGraphicsWindow());
	viewer->getCamera()->setViewport(0, 0, size.GetWidth(), size.GetHeight());
	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
	viewer->setCameraManipulator(new osgGA::TrackballManipulator);
	//viewer->setSceneData(scn.get_root());
	
	refresh_timer.Start(REFRESH_INTERVAL);
}

void MainFrame::on_refresh(wxTimerEvent &event) {
	if (!viewer->isRealized()) {
		return;
	}
	
	read_nonblock(input_buf);
	int end = input_buf.find('\n');
	if (end != string::npos) {
		string line = input_buf.substr(0, end);
		input_buf.erase(0, end + 1);
		parse(line);
	}
	viewer->frame();
}

void MainFrame::add_scene(const string &name) {
	assert(find_if(scns.begin(), scns.end(), [&name](const scene_stuff &m) { return m.name == name; }) == scns.end());
	
	scene_stuff s(name, new scene(), new SceneChangeButton(this, name));
	btn_sizer->Add(s.btn, 1, 0, 0);
	scns.push_back(s);
	set_scene(name);
}

void MainFrame::del_scene(const string &name) {
	auto i = find_if(scns.begin(), scns.end(), [&name](const scene_stuff &m) { return m.name == name; });
	assert(i != scns.end());
	btn_sizer->Detach(i->btn);
	i->btn->Destroy();
	
	if (scns.size() == 1) {
		scns.erase(i);
		set_scene("");
	} else if (i == scns.begin()) {
		scns.erase(i);
		set_scene(scns.front().name);
	} else {
		set_scene(prev(i)->name);
		scns.erase(i);
	}
}

void MainFrame::set_scene(const string &s) {
	if (s.empty()) {
		curr_scn.reset();
		viewer->setSceneData(NULL);
		cout << "Setting scene to empty" << endl;
	} else {
		for (auto& i : scns) {
			if (i.name == s) {
				curr_scn = i.scn;
				viewer->setSceneData(curr_scn->get_root());
				cout << "Setting scene to " << s << endl;
				return;
			}
		}
		assert(false);
	}
}

void MainFrame::parse(const string &line) {
	string whitespace = " \t\n";
	int e1 = line.find_first_of(whitespace);
	int b2 = line.find_first_not_of(whitespace, e1 + 1);
	int e2 = line.find_first_of(whitespace, e2 + 1);
	
	string first = line.substr(0, e1), second = line.substr(b2, e2 - b2);
	for (auto& i : scns) {
		if (i.name == first) {
			if (second == "delete") {
				del_scene(first);
			} else {
				i.scn->update(line.substr(b2));
			}
			return;
		}
	}
	
	// scene doesn't exist, add it
	add_scene(first);
	assert(second != "delete");
	scns.back().scn->update(line.substr(b2));
}

BEGIN_EVENT_TABLE(SceneChangeButton, wxButton)
	EVT_LEFT_UP (SceneChangeButton::on_click)
END_EVENT_TABLE()

SceneChangeButton::SceneChangeButton(MainFrame *parent, const string &scn)
: wxButton(parent, wxID_ANY), frm(parent), scn(scn)
{
	SetLabel(wxString(scn.c_str(), wxConvUTF8));
}

void SceneChangeButton::on_click(wxMouseEvent &evt) {
	frm->set_scene(scn);
	evt.Skip();
}

BEGIN_EVENT_TABLE(OSGCanvas, wxGLCanvas)
	EVT_SIZE                (OSGCanvas::OnSize)
	EVT_PAINT               (OSGCanvas::OnPaint)
	EVT_ERASE_BACKGROUND    (OSGCanvas::OnEraseBackground)

	EVT_CHAR                (OSGCanvas::OnChar)
	EVT_KEY_UP              (OSGCanvas::OnKeyUp)

	EVT_ENTER_WINDOW        (OSGCanvas::OnMouseEnter)
	EVT_LEFT_DOWN           (OSGCanvas::OnMouseDown)
	EVT_MIDDLE_DOWN         (OSGCanvas::OnMouseDown)
	EVT_RIGHT_DOWN          (OSGCanvas::OnMouseDown)
	EVT_LEFT_UP             (OSGCanvas::OnMouseUp)
	EVT_MIDDLE_UP           (OSGCanvas::OnMouseUp)
	EVT_RIGHT_UP            (OSGCanvas::OnMouseUp)
	EVT_MOTION              (OSGCanvas::OnMouseMotion)
	EVT_MOUSEWHEEL          (OSGCanvas::OnMouseWheel)
END_EVENT_TABLE()

OSGCanvas::OSGCanvas(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name, int *attributes)
: wxGLCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name, attributes)
{
	gw = new GraphicsWindowWX(this);
	// default cursor to standard
	_oldCursor = *wxSTANDARD_CURSOR;
}

void OSGCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) ) {
	/* must always be here */
	wxPaintDC dc(this);
}

void OSGCanvas::OnSize(wxSizeEvent& event) {
	// this is also necessary to update the context on some platforms
	wxGLCanvas::OnSize(event);

	// set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
	int width, height;
	GetClientSize(&width, &height);

	if (gw.valid()) {
		// update the window dimensions, in case the window has been resized.
		gw->getEventQueue()->windowResize(0, 0, width, height);
		gw->resized(0,0,width,height);
	}
}

void OSGCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event)) {
	/* Do nothing, to avoid flashing on MSW */
}

void OSGCanvas::OnChar(wxKeyEvent &event)
{
#if wxUSE_UNICODE
	int key = event.GetUnicodeKey();
#else
	int key = event.GetKeyCode();
#endif

	if (gw.valid()) {
		gw->getEventQueue()->keyPress(key);
	}

	// If this key event is not processed here, we should call
	// event.Skip() to allow processing to continue.
}

void OSGCanvas::OnKeyUp(wxKeyEvent &event)
{
#if wxUSE_UNICODE
	int key = event.GetUnicodeKey();
#else
	int key = event.GetKeyCode();
#endif

	if (gw.valid()) {
		gw->getEventQueue()->keyRelease(key);
	}

	// If this key event is not processed here, we should call
	// event.Skip() to allow processing to continue.
}

void OSGCanvas::OnMouseEnter(wxMouseEvent &event)
{
	// Set focus to ourselves, so keyboard events get directed to us
	SetFocus();
}

void OSGCanvas::OnMouseDown(wxMouseEvent &event) {
	if (gw.valid()) {
		gw->getEventQueue()->mouseButtonPress(event.GetX(), event.GetY(), event.GetButton());
	}
}

void OSGCanvas::OnMouseUp(wxMouseEvent &event) {
	if (gw.valid()) {
		gw->getEventQueue()->mouseButtonRelease(event.GetX(), event.GetY(),
			event.GetButton());
	}
}

void OSGCanvas::OnMouseMotion(wxMouseEvent &event) {
	if (gw.valid()) {
		gw->getEventQueue()->mouseMotion(event.GetX(), event.GetY());
	}
}

void OSGCanvas::OnMouseWheel(wxMouseEvent &event)
{
	int delta = event.GetWheelRotation() / event.GetWheelDelta() * event.GetLinesPerAction();

	if (gw.valid()) {
		gw->getEventQueue()->mouseScroll( delta>0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN);
	}
}

void OSGCanvas::UseCursor(bool value)
{
	if (value)
	{
		// show the old cursor
		SetCursor(_oldCursor);
	}
	else
	{
		// remember the old cursor
		_oldCursor = GetCursor();

		// hide the cursor
		//    - can't find a way to do this neatly, so create a 1x1, transparent image
		wxImage image(1,1);
		image.SetMask(true);
		image.SetMaskColour(0, 0, 0);
		wxCursor cursor(image);
		SetCursor(cursor);

		// On wxGTK, only works as of version 2.7.0
		// (http://trac.wxwidgets.org/ticket/2946)
		// SetCursor( wxStockCursor( wxCURSOR_BLANK ) );
	}
}

GraphicsWindowWX::GraphicsWindowWX(OSGCanvas *canvas)
{
	_canvas = canvas;

	_traits = new GraphicsContext::Traits;

	wxPoint pos = _canvas->GetPosition();
	wxSize  size = _canvas->GetSize();

	_traits->x = pos.x;
	_traits->y = pos.y;
	_traits->width = size.x;
	_traits->height = size.y;

	init();
}

GraphicsWindowWX::~GraphicsWindowWX()
{
}

void GraphicsWindowWX::init()
{
	if (valid())
	{
		setState( new osg::State );
		getState()->setGraphicsContext(this);

		if (_traits.valid() && _traits->sharedContext)
		{
			getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
			incrementContextIDUsageCount( getState()->getContextID() );
		}
		else
		{
			getState()->setContextID( osg::GraphicsContext::createNewContextID() );
		}
	}
}

void GraphicsWindowWX::grabFocus()
{
	// focus the canvas
	_canvas->SetFocus();
}

void GraphicsWindowWX::grabFocusIfPointerInWindow()
{
	// focus this window, if the pointer is in the window
	wxPoint pos = wxGetMousePosition();
	if (wxFindWindowAtPoint(pos) == _canvas)
		_canvas->SetFocus();
}

void GraphicsWindowWX::useCursor(bool cursorOn)
{
	_canvas->UseCursor(cursorOn);
}

bool GraphicsWindowWX::makeCurrentImplementation()
{
	_canvas->SetCurrent();
	return true;
}

void GraphicsWindowWX::swapBuffersImplementation()
{
	_canvas->SwapBuffers();
}

