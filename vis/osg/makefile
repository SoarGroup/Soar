FLAGS=-std=c++0x -L/home/jzxu/local/lib `wx-config --libs --cxxflags --gl-libs`
LIBS=-losg -losgViewer -losgFX -lOpenThreads -losgGA -losgText

osgvis: osgviewerWX.cpp osgviewerWX.h scene.cpp scene.h
	g++ $(FLAGS) -o osgvis osgviewerWX.cpp scene.cpp $(LIBS)
	
