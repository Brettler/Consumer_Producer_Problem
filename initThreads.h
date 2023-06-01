#pragma once
#ifndef TASK3_INITTHREADS_H
#define TASK3_INITTHREADS_H
#include "Structs.h"
pthread_t* createProducerThreads(Producer** ArrayProducers, Config* config);
pthread_t createDispatcherThread(Dispatcher* dispatcher);
pthread_t* createCoEditorThreads(Dispatcher* dispatcher, BoundedBuffer* SharedBuffer, CoEditor*** pArrayCoEditors);
pthread_t createManagerThread(Manager* manager);
#endif //TASK3_INITTHREADS_H
