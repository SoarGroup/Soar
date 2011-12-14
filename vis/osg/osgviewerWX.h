#ifndef _WXSIMPLEVIEWERWX_H_
#define _WXSIMPLEVIEWERWX_H_

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/cursor.h"
#include "wx/glcanvas.h"
#include <osgViewer/Viewer>
#include <string>
#include "scene.h"

class GraphicsWindowWX;

class OSGCanvas : public wxGLCanvas {
public:
	OSGCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0,
		const wxString& name = wxT("TestGLCanvas"),
		int *attributes = 0);

	virtual ~OSGCanvas() {}

	osgViewer::GraphicsWindow* GetGraphicsWindow() { return gw.get(); }

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);

	void OnChar(wxKeyEvent &event);
	void OnKeyUp(wxKeyEvent &event);

	void OnMouseEnter(wxMouseEvent &event);
	void OnMouseDown(wxMouseEvent &event);
	void OnMouseUp(wxMouseEvent &event);
	void OnMouseMotion(wxMouseEvent &event);
	void OnMouseWheel(wxMouseEvent &event);

	void UseCursor(bool value);

private:
	DECLARE_EVENT_TABLE()

	osg::ref_ptr<osgViewer::GraphicsWindow> gw;

	wxCursor _oldCursor;
};

class GraphicsWindowWX : public osgViewer::GraphicsWindow
{
public:
	GraphicsWindowWX(OSGCanvas *canvas);
	~GraphicsWindowWX();

	void init();

	//
	// GraphicsWindow interface
	//
	void grabFocus();
	void grabFocusIfPointerInWindow();
	void useCursor(bool cursorOn);

	bool makeCurrentImplementation();
	void swapBuffersImplementation();

	// not implemented yet...just use dummy implementation to get working.
	virtual bool valid() const { return true; }
	virtual bool realizeImplementation() { return true; }
	virtual bool isRealizedImplementation() const  { return _canvas->IsShownOnScreen(); }
	virtual void closeImplementation() {}
	virtual bool releaseContextImplementation() { return true; }

private:
	// XXX need to set _canvas to NULL when the canvas is deleted by
	// its parent. for this, need to add event handler in OSGCanvas
	OSGCanvas*  _canvas;
};

class SceneChangeButton;

class MainFrame : public wxFrame {
public:
	MainFrame(const wxSize &size);
	void on_refresh(wxTimerEvent& evt);
	
	void parse(const std::string &line);
	void add_scene(const std::string &curr);
	void del_scene(const std::string &curr);
	void set_scene(const std::string &curr);

private:
	osg::ref_ptr<osgViewer::Viewer> viewer;
	OSGCanvas* canvas;  // can't use unique_ptr, will mess up wxWidgets deallocation
	
	std::string input_buf;
	std::shared_ptr<scene> curr_scn;
	
	struct scene_stuff {
		std::string name;
		std::shared_ptr<scene> scn;
		SceneChangeButton *btn;
		
		scene_stuff(const std::string &name, scene *scn, SceneChangeButton *btn)
		: name(name), scn(scn), btn(btn)
		{}
	};
	
	std::list<scene_stuff> scns;
	
	wxBoxSizer *canvas_sizer, *btn_sizer;
	
	wxTimer refresh_timer;

	DECLARE_EVENT_TABLE()
};

class wxOsgApp : public wxApp {
	bool OnInit();
	void on_click(wxCommandEvent &evt);
};

class SceneChangeButton : public wxButton {
public:
	SceneChangeButton(MainFrame *parent, const std::string &scn);
	void on_click(wxMouseEvent &evt);
	
private:
	MainFrame *frm;
	std::string scn;
	
	DECLARE_EVENT_TABLE()
};

#endif // _WXSIMPLEVIEWERWX_H_
