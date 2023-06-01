#include "initThreads.h"

// This function creates a thread for each producer
pthread_t* createProducerThreads(Producer** ArrayProducers, Config* config) {
    pthread_t* ProducerThreads = malloc(config -> TotalNumProducers * sizeof(pthread_t));
    for (int i = 0; i < config -> TotalNumProducers; ++i) {
        // Create a new thread that will execute the producerThread function with the current producer as argument
        pthread_create(&ProducerThreads[i], NULL, producerThread, ArrayProducers[i]);
    }
    return ProducerThreads;
}

// This function creates a thread for the dispatcher
pthread_t createDispatcherThread(Dispatcher* dispatcher) {
    pthread_t dispatcher_thread_id;
    // Create a new thread that will execute the dispatcherThread function with the dispatcher as argument
    pthread_create(&dispatcher_thread_id, NULL, dispatcherThread, dispatcher);
    return dispatcher_thread_id;
}

// This function creates a thread for each co-editor
pthread_t* createCoEditorThreads(Dispatcher* dispatcher, BoundedBuffer* SharedBuffer, CoEditor*** pArrayCoEditors) {
    // Allocate memory for an array of pthreads and an array of CoEditors
    pthread_t* coEditorThreads = malloc(NUM_ARTICLES_TYPES * sizeof(pthread_t));
    CoEditor** ArrayCoEditors = malloc(NUM_ARTICLES_TYPES * sizeof(CoEditor*));
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        ArrayCoEditors[i] = malloc(sizeof(CoEditor));
        ArrayCoEditors[i] -> dispatcherBuffer = dispatcher -> DispatcherBuffersArray[i];
        ArrayCoEditors[i] -> SharedBuffer = SharedBuffer;
        // Create a new thread that will execute the coEditorThread function with the current co-editor as argument
        pthread_create(&coEditorThreads[i], NULL, coEditorThread, ArrayCoEditors[i]);
    }
    *pArrayCoEditors = ArrayCoEditors; // Set the passed pointer to point to ArrayCoEditors
    return coEditorThreads;
}

// This function creates a thread for the manager
pthread_t createManagerThread(Manager* manager) {
    // Create manager thread
    pthread_t ManagerThreadID;
    // Create a new thread that will execute the managerThread function with the manager as argument
    pthread_create(&ManagerThreadID, NULL, managerThread, manager);
    return ManagerThreadID;
}