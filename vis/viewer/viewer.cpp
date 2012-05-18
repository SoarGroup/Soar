/*
 Adapted from examples/osgviewerSDL/osgviewerSDL.cpp
*/
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <pthread.h>
#include <GL/glut.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include "scene.h"

using namespace std;

class scene_manager {
public:
	scene_manager(osg::ref_ptr<osgViewer::Viewer> &v, int scene_menu_id) 
	: viewer(v), scene_menu_id(scene_menu_id), current(-1)
	{}
	
	~scene_manager() {
		for (int i = 0; i < scenes.size(); ++i) {
			delete scenes[i].second;
		}
	}

	void add_scene(const string &name) {
		for(int i = 0; i < scenes.size(); ++i) {
			if (scenes[i].first == name) {
				cerr << "scene already exists: " << name << endl;
				return;
			}
		}
		scenes.push_back(make_pair(name, new scene()));
		set_scene(-1);
		
		glutSetMenu(scene_menu_id);
		glutAddMenuEntry(scenes[scenes.size()-1].first.c_str(), scenes.size() - 1);
	}
	
	void del_scene(const string &name) {
		int i;
		for (i = 0; i < scenes.size(); ++i) {
			if (scenes[i].first == name) {
				break;
			}
		}
		if (i == scenes.size()) {
			cerr << "scene does not exist: " << name << endl;
			return;
		}
		del_scene(i);
	}
		
	void del_scene(int i) {
		assert(i >= 0 && i < scenes.size());
		
		delete scenes[i].second;
		for (int j = i + 1; j < scenes.size(); ++j) {
			scenes[j - 1] = scenes[j];
		}
		scenes.resize(scenes.size() - 1);
		
		if (scenes.empty()) {
			set_scene(0);
		} else if (i >= scenes.size()) {
			set_scene(-1);
		} else {
			set_scene(i);
		}
		
		glutSetMenu(scene_menu_id);
		// remove lower menu items and rebuild
		for (int j = scenes.size(); j >= i; --j) {
			glutRemoveMenuItem(j + 2);
		}
		for (int j = i; j < scenes.size(); ++j) {
			glutAddMenuEntry(scenes[j].first.c_str(), j);
		}
	}
	
	void set_scene(int i) {
		if (i < 0) {
			i = scenes.size() + i;
		}
		if (i >= 0 && i < scenes.size()) {
			viewer->setSceneData(scenes[i].second->get_root());
			current = i;
		} else {
			viewer->setSceneData(NULL);
			current = -1;
		}
	}
	
	void parse(const string &line) {
		string whitespace = " \t\n";
		int e1 = line.find_first_of(whitespace);
		int b2 = line.find_first_not_of(whitespace, e1 + 1);
		int e2 = line.find_first_of(whitespace, e2 + 1);
		
		string f1 = line.substr(0, e1), f2 = line.substr(b2, e2 - b2);
		if (f2 == "delete") {
			del_scene(f1);
		}
		
		int i;
		for (i = 0; i < scenes.size(); ++i) {
			if (scenes[i].first == f1) {
				break;
			}
		}
		if (i == scenes.size()) {
			add_scene(f1);
		}
		scenes[i].second->update(line.substr(b2));
		//viewer->frame();
	}
	
	void clear() {
		while (scenes.size() > 0) {
			del_scene(scenes.size() - 1);
		}
	}
	
private:
	osg::ref_ptr<osgViewer::Viewer> viewer;
	vector<pair<string, scene*> > scenes;

	int current;
	int scene_menu_id;
	
	string input;
};

vector<string> read_buf;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
bool finished = false;

osg::ref_ptr<osgViewer::Viewer> viewer;
osg::observer_ptr<osgViewer::GraphicsWindow> window;
scene_manager *scn_mgr;

void *read_stdin(void *ptr) {
	string line;
	while (getline(cin, line)) {
		if (line.find_first_not_of("\t\n ") != string::npos) {
			pthread_mutex_lock(&mut);
			read_buf.push_back(line);
			pthread_mutex_unlock(&mut);
		}
	}
	finished = true;
}

void display(void) {
	pthread_mutex_lock(&mut);
	for (int i = 0; i < read_buf.size(); ++i) {
		scn_mgr->parse(read_buf[i]);
	}
	read_buf.clear();
	pthread_mutex_unlock(&mut);
	
	if (viewer.valid()) {
		viewer->frame();
	}
	glutSwapBuffers();
	glutPostRedisplay();
	
	if (finished) {
		glutDestroyWindow(glutGetWindow());
	}
}

void reshape( int w, int h ) {
	// update the window dimensions, in case the window has been resized.
	if (window.valid()) {
		window->resized(window->getTraits()->x, window->getTraits()->y, w, h);
		window->getEventQueue()->windowResize(window->getTraits()->x, window->getTraits()->y, w, h );
	}
}

void mousebutton( int button, int state, int x, int y ) {
	if (window.valid()) {
		if (state==0) {
			switch (button) {
			case 3:
				window->getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_UP);
				break;
			case 4:
				window->getEventQueue()->mouseScroll(osgGA::GUIEventAdapter::SCROLL_DOWN);
			default:
				window->getEventQueue()->mouseButtonPress( x, y, button+1 );
			}
		} else {
			window->getEventQueue()->mouseButtonRelease( x, y, button+1 );
		}
	}
}

void mousemove( int x, int y ) {
	if (window.valid()) {
		window->getEventQueue()->mouseMotion( x, y );
	}
}

void keyboard( unsigned char key, int /*x*/, int /*y*/ )
{
	if (window.valid()) {
		window->getEventQueue()->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
		window->getEventQueue()->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
	}
}

enum MenuID {
	MENU_RESET,
	MENU_WIREFRAME,
	MENU_QUIT,
	MENU_END,
};

struct MenuDef {
	MenuID id;
	const char *label;
};

MenuDef menu_definition[] = {
	{ MENU_RESET,     "reset camera" },
	{ MENU_WIREFRAME, "toggle wireframe" },
	{ MENU_QUIT,      "quit" },
	{ MENU_END,       NULL},
};

void top_menu_callback(int value) {
	switch (value) {
	case MENU_RESET:
		viewer->home();
		break;
	case MENU_QUIT:
		glutDestroyWindow(glutGetWindow());
		break;
	}
}

void scene_menu_callback(int value) {
	cout << "scene menu: " << value << endl;
	scn_mgr->set_scene(value);
}

int create_menu() {
	int scene_menu_id = glutCreateMenu(&scene_menu_callback);
	glutAddMenuEntry("bottom", -1);
	
	int menu_id = glutCreateMenu(&top_menu_callback);
	glutAddSubMenu("show scene", scene_menu_id);
	for (int i = 0; menu_definition[i].id != MENU_END; ++i) {
		glutAddMenuEntry(menu_definition[i].label, menu_definition[i].id);
	}
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	return scene_menu_id;
}

int main( int argc, char **argv ) {
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA );
	glutInitWindowPosition( 100, 100 );
	glutInitWindowSize( 800, 600 );
	glutCreateWindow( argv[0] );
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutMouseFunc( mousebutton );
	glutMotionFunc( mousemove );
	glutKeyboardFunc( keyboard );
	
	viewer = new osgViewer::Viewer;
	window = viewer->setUpViewerAsEmbeddedInWindow(0, 0, 800, 600);
	viewer->setCameraManipulator(new osgGA::TrackballManipulator());
	viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->realize();
	
	int scene_menu_id = create_menu();
	
	scn_mgr = new scene_manager(viewer, scene_menu_id);
	
	pthread_t read_thread;
	pthread_create( &read_thread, NULL, &read_stdin, NULL);

	glutMainLoop();

	return 0;
}

/*EOF*/
