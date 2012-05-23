/*
 Adapted from examples/osgviewerSDL/osgviewerSDL.cpp
*/
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <pthread.h>
#include <GL/glut.h>

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include "scene.h"

using namespace std;

const int BUFFERSIZE = 10240;
const char *DEFAULT_PATH = "/tmp/viewer";

void split(const string &s, const string &delim, vector<string> &fields) {
	int start, end = 0;
	fields.clear();
	while (end < s.size()) {
		start = s.find_first_not_of(delim, end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(delim, start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
	}
}

class sock {
public:
	sock(int port) {
		struct sockaddr_in addr;
		
		bzero((char *) &addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}
		if (bind(listenfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
			perror("socket");
			exit(1);
		}
		if (::listen(listenfd, 1) == -1) {
			perror("socket");
			exit(1);
		}
		len = sizeof(struct sockaddr_in);
	}
	
	sock(const char *path) {
		socklen_t len;
		struct sockaddr_un addr;
		
		bzero((char *) &addr, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, path);
		
		unlink(addr.sun_path);
		if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}
		socklen_t l = sizeof(addr.sun_family) + strlen(addr.sun_path) + 1;
		if (bind(listenfd, (struct sockaddr *) &addr, l) == -1) {
			perror("socket");
			exit(1);
		}
		if (::listen(listenfd, 1) == -1) {
			perror("socket");
			exit(1);
		}
		len = sizeof(struct sockaddr_un);
	}
	
	~sock() {
		close(fd);
	}
	
	bool listen() {
		struct sockaddr_in remote;
		if ((fd = ::accept(listenfd, (struct sockaddr *) &remote, &len)) == -1) {
			perror("socket");
			return false;
		}
		return true;
	}
	
	bool recv_line(string &line) {
		char buf[BUFFERSIZE+1];
		
		while(true) {
			size_t i = recvbuf.find('\n'), n;
			if (i != string::npos) {
				line = recvbuf.substr(0, i);
				recvbuf = recvbuf.substr(i + 1);
				return true;
			}
			if ((n = recv(fd, buf, BUFFERSIZE, 0)) <= 0) {
				return false;
			}
			buf[n] = '\0';
			recvbuf += buf;
		}
	}
	
private:
	string recvbuf;
	int listenfd, fd;
	socklen_t len;
};

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
		vector<string> fields;
		split(line, " \t\n", fields);
		
		if (fields.empty()) {
			return;
		}
		
		if (fields[0] == "clear") {
			clear();
		}
		
		if (fields[1] == "delete") {
			del_scene(fields[0]);
		}
		
		int i;
		for (i = 0; i < scenes.size(); ++i) {
			if (scenes[i].first == fields[0]) {
				break;
			}
		}
		if (i == scenes.size()) {
			add_scene(fields[0]);
		}
		
		for (int j = 1; j < fields.size(); ++j) {
			fields[j - 1] = fields[j];
		}
		fields.resize(fields.size() - 1);
		
		scenes[i].second->update(fields);
	}
	
	void clear() {
		while (scenes.size() > 0) {
			del_scene(scenes.size() - 1);
		}
	}
	
	void toggle_axes() {
		for (int i = 0; i < scenes.size(); ++i) {
			scenes[i].second->toggle_axes();
		}
	}
	
private:
	osg::ref_ptr<osgViewer::Viewer> viewer;
	vector<pair<string, scene*> > scenes;

	int current;
	int scene_menu_id;
	
	string input;
};

sock *sck;
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

void *read_socket(void *ptr) {
	string line;
	
	if (!sck) {
		finished = true;
		return NULL;
	}
	while (sck->listen()) {
		cerr << "connect" << endl;
		while (sck->recv_line(line)) {
			if (line.find_first_not_of("\t\n ") != string::npos) {
				cerr << line << endl;
				pthread_mutex_lock(&mut);
				read_buf.push_back(line);
				pthread_mutex_unlock(&mut);
			}
		}
		cerr << "disconnect" << endl;
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
	MENU_AXES,
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
	{ MENU_AXES,      "toggle axes" },
	{ MENU_QUIT,      "quit" },
	{ MENU_END,       NULL},
};

void top_menu_callback(int value) {
	switch (value) {
	case MENU_RESET:
		viewer->home();
		break;
	case MENU_AXES:
		scn_mgr->toggle_axes();
		break;
	case MENU_QUIT:
		glutDestroyWindow(glutGetWindow());
		exit(0);
		break;
	}
}

void scene_menu_callback(int value) {
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

/*
 This program can accept input in one of three ways:
 1. standard input
 2. TCP socket using a port
 3. TCP socket using a file (AKA unix domain socket)
 
 #3 is the default method, but the user can change this with command
 line options that we parse here.
*/
void parse_conn_args(int argc, char *argv[]) {
	int port = -1;
	const char *sock_path = DEFAULT_PATH;
	
	for (int i = 1; i < argc; ++i) {
		if (strcmp("-s", argv[i]) == 0) {
			port = -1;
			sock_path = NULL;
			break;
		} else if (strcmp("-p", argv[i]) == 0) {
			// open a port
			if (i >= argc - 1) {
				cerr << "specify a port" << endl;
				exit(1);
			}
			
			char *end;
			port = strtol(argv[++i], &end, 10);
			if (*end != '\0') {
				cerr << "invalid port number" << endl;
				exit(1);
			}
			break;
		} else if (strcmp("-f", argv[i]) == 0) {
			// create a socket file
			if (i >= argc - 1) {
				cerr << "specify a file" << endl;
				exit(1);
			}
			sock_path = argv[++i];
			break;
		}
	}
	
	if (port >= 0) {
		sck = new sock(port);
	} else if (sock_path != NULL) {
		sck = new sock(sock_path);
	} else {
		// will use stdin
		sck = NULL;
	}
}

int main( int argc, char **argv ) {
	parse_conn_args(argc, argv);
	
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
	if (!sck) {
		pthread_create( &read_thread, NULL, &read_stdin, NULL);
	} else {
		pthread_create( &read_thread, NULL, &read_socket, NULL);
	}

	glutMainLoop();

	return 0;
}

/*EOF*/
