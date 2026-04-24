#pragma once

#include "CmEngine.h"
#include "CmApplication.h"

extern chimi::CmApplication *CreateApplicationEntryPoint();

#if CHIMI_ENGINE_PLATFORM_WIN32 || CHIMI_ENGINE_PLATFORM_MACOS || CHIMI_ENGINE_PLATFORM_LINUX

int main(int argc, char *argv[]){

    std::cout<< "Chimi Engine starting..." << std::endl;

    chimi::CmApplication *app = CreateApplicationEntryPoint();
    // start
    app->Start(argc, argv);
    // main loop
    app->MainLoop();
    // stop
    app->Stop();
    // delete
    delete app;

    return EXIT_SUCCESS;
}

#endif
