#include "testApp.h"
#include "bounce.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
    //window.setGlutDisplayString("rgb alpha double samples depth");
	ofSetupOpenGL(&window, 1280, 720, OF_WINDOW);
	//ofRunApp(new testApp());
    ofRunApp(new bounce());
}
