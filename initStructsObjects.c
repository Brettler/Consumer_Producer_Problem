#include "initStructsObjects.h"

// This function creates an array of producers as per the configuration provided
Producer** createProducers(Config* config) {
    Producer** ArrayProducers = malloc(config -> TotalNumProducers * sizeof(Producer*));
    // Print the values in config
    for (int i = 0; i < config -> TotalNumProducers; ++i) {
        ArrayProducers[i] = malloc(sizeof(Producer)); // Allocate memory for each producer
        ArrayProducers[i] -> id = config -> ArrayProducers[i].id; // Assign each producer its ID from configuration
        ArrayProducers[i] -> NumArticles = config -> ArrayProducers[i].NumArticles;
        // Create a bounded buffer for each producer with the specified queue size
        // We create here in the main so the dispatcher wil have acess to it.
        ArrayProducers[i]-> ProducerBuffer = constructorBoundedBuffer(config -> ArrayProducers[i].QueueLength);
    }
    return ArrayProducers;
}

// This function creates an array of unbounded buffers for the dispatcher.
UnboundedBuffer** createDispatcherBuffers() {
    UnboundedBuffer** DispatcherBuffersArray = malloc(NUM_ARTICLES_TYPES * sizeof(UnboundedBuffer*));
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        // Create a bounded buffer for each type of message with the specified queue length.
        DispatcherBuffersArray[i] = constructorUnboundedBuffer();
    }
    return DispatcherBuffersArray;
}

// This function creates a Dispatcher.
Dispatcher* createDispatcher(Producer** ArrayProducers, Config* config, UnboundedBuffer** DispatcherBuffersArray) {
    // Allocate memory for a Dispatcher and declare it.
    Dispatcher* dispatcher = malloc(sizeof(Dispatcher));
    dispatcher -> producers = ArrayProducers;
    dispatcher -> TotalNumProducers = config -> TotalNumProducers;
    dispatcher -> DispatcherBuffersArray = DispatcherBuffersArray;
    return dispatcher;
}

// This function creates a Manager using the provided shared buffer and config.
Manager* createManager(BoundedBuffer* SharedBuffer, Config* config) {
    // Allocate memory for Struct manager and declare it.
    Manager* manager = malloc(sizeof(Manager));
    manager -> SharedBuffer = SharedBuffer;
    manager -> doneCount = 0;
    manager -> TotalNumProducers = config -> TotalNumProducers;
    return manager;
}