#include "Structs.h"
char* types[NUM_ARTICLES_TYPES] = {"Sports", "News", "Weather"};

void joinThreads(pthread_t* ProducerThreads, pthread_t dispatcher_thread_id, pthread_t* coEditorThreads, pthread_t ManagerThreadID, Config* config) {
    for (int i = 0; i < config -> TotalNumProducers; ++i) {
        pthread_join(ProducerThreads[i], NULL);
    }
    pthread_join(dispatcher_thread_id, NULL);
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        pthread_join(coEditorThreads[i], NULL);
    }
    pthread_join(ManagerThreadID, NULL);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Wrong number of variables\n");
        return 1;
    }

    // Parse configuration file
    Config* config = processConfig(argv[1]);
    if (config == NULL) {
        return 1;
    }

    // Create array of Producer pointers, one for each producer specified in the config file
    Producer** ArrayProducers = createProducers(config);
    // Create an array of BoundedBuffer pointers, one for each type of message
    UnboundedBuffer** DispatcherBuffersArray = createDispatcherBuffers();
    // Create a dispatcher and assign the producers to it
    Dispatcher* dispatcher = createDispatcher(ArrayProducers, config, DispatcherBuffersArray);
    // Create a thread for each producer
    pthread_t* ProducerThreads = createProducerThreads(ArrayProducers, config);
    // Create a thread for the dispatcher with set the function
    pthread_t dispatcher_thread_id = createDispatcherThread(dispatcher);
    // Create a shared bounded buffer for the co-editors and the manager
    BoundedBuffer* SharedBuffer = constructorBoundedBuffer(config -> QueueLengthCoEditor);
    // Create co-editor threads
    CoEditor** ArrayCoEditors;
    pthread_t* coEditorThreads = createCoEditorThreads(dispatcher, SharedBuffer, &ArrayCoEditors);
    // Allocate memory for Struct manager and declare it.
    Manager* manager = createManager(SharedBuffer, config);
    // Create manager thread
    pthread_t ManagerThreadID = createManagerThread(manager);
    // Join threads
    joinThreads(ProducerThreads, dispatcher_thread_id, coEditorThreads, ManagerThreadID, config);
    // Clean up
    FreeResources(config, ArrayCoEditors, coEditorThreads, manager, SharedBuffer, DispatcherBuffersArray, dispatcher);
    // PLEASE EXECUTE THE FOLLOWING LINE: (THANK YOU).
    return 0;
}
