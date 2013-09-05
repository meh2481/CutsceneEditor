/*
    CutsceneEditor source - main.cpp
    Copyright (c) 2013 Mark Hutcheson
*/

#include "cutsceneEngine.h"

#ifdef _WIN32
#undef main
#endif
int main(int argc, char *argv[])
{
    errlog << "Starting program" << endl;
#ifdef __APPLE__
    errlog << "Starting FreeImage" << endl;
    FreeImage_Initialise();
#endif
    CutsceneEngine* eng = new CutsceneEngine(800, 600, "Cutscene Editor", true); //Create our engine

    eng->setFramerate(60);
	eng->commandline(argc, argv);
    eng->start(); //Get the engine rolling
	errlog << "Deleting engine" << endl;
    delete eng;
#ifdef __APPLE__
    errlog << "Closing FreeImage" << endl;
    FreeImage_DeInitialise();
#endif
    errlog << "Ending program happily" << endl;
    return 0;
}
