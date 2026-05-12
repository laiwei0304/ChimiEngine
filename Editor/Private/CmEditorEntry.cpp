#include "CmEntryPoint.h"
#include "CmEditorApp.h"

chimi::CmApplication *CreateApplicationEntryPoint(){
    return new chimi::CmEditorApp();
}
