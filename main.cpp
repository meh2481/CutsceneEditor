/*
    CutsceneEditor source - main.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "cutsceneEngine.h"

#ifdef _WIN32
#define ICONNAME "res/icon.png"
#else
#define ICONNAME "res/icon.ico"
#endif

int main(int argc, char *argv[])
{
    errlog << "Starting program" << endl;
    errlog << "Starting FreeImage" << endl;
    FreeImage_Initialise();
    CutsceneEngine* eng = new CutsceneEngine(800, 600, "Cutscene Editor", ICONNAME, true); //Create our engine

    eng->setFramerate(60);
	eng->commandline(argc, argv);
    eng->start(); //Get the engine rolling
	errlog << "Deleting engine" << endl;
    delete eng;
    errlog << "Closing FreeImage" << endl;
    FreeImage_DeInitialise();
    errlog << "Ending program happily" << endl;
    return 0;
}
