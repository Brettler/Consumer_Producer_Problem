#ifndef TASK3_INITSTRUCTSOBJECTS_H
#define TASK3_INITSTRUCTSOBJECTS_H
#include "Structs.h"
Producer** createProducers(Config* config);
UnboundedBuffer** createDispatcherBuffers();
Dispatcher* createDispatcher(Producer** ArrayProducers, Config* config, UnboundedBuffer** DispatcherBuffersArray);
Manager* createManager(BoundedBuffer* SharedBuffer, Config* config);
#endif //TASK3_INITSTRUCTSOBJECTS_H
