#pragma once
#ifndef TASK3_FREERESOURCE_H
#define TASK3_FREERESOURCE_H
#include "Structs.h"
void FreeResources(Config *config, CoEditor **ArrayCoEditors, pthread_t *coEditorThreads, Manager *manager,
                   BoundedBuffer *SharedBuffer, UnboundedBuffer **DispatcherBuffersArray, Dispatcher *dispatcher);
#endif //TASK3_FREERESOURCE_H
