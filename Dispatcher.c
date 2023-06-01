# include "Dispatcher.h"

// This function processes a "DONE" message from a producer. It cleans up the producer and increments the done_count.
// If all producers are done, it sends "DONE" through each dispatcher's queue.
void processDoneMessage(Dispatcher* dispatcher, int producerIndex, int* done_count) {
    // Terminate and clean up this producer
    destructorBoundedBuffer(dispatcher -> producers[producerIndex] -> ProducerBuffer);
    free(dispatcher -> producers[producerIndex]); // Free the producer struct
    dispatcher -> producers[producerIndex] = NULL; // Set the pointer in the producers array to NULL

    // Increment the counter how much times we saw done. in the third time it will
    (*done_count)++;

    // All producers are done, send "DONE" through each dispatcher's queue and exit loop
    if (*done_count == (dispatcher -> TotalNumProducers)) {
        for (int j = 0; j < NUM_ARTICLES_TYPES; ++j) {
            insertUnboundedBuffer(dispatcher->DispatcherBuffersArray[j], "DONE");
        }
    }
}

int checkTypeArticle(char* message) {
    if (strstr(message, "Sports") != NULL) {
        return 0;
    } else if (strstr(message, "News") != NULL) {
        return 1;
    } else if (strstr(message, "Weather") != NULL) {
        return 2;
    }
    return -1; // error, unknown type
}

// This function processes a normal message from a producer. It parses the message to determine its type,
// then inserts the message into the appropriate dispatcher's queue (for the co-editor usage).
void sendToCoEditor(Dispatcher* dispatcher, char* message) {
    int type_index = checkTypeArticle(message);
    if (type_index == -1) {
        perror("Invalid Message Type");
        exit(-1);
    }
    // Insert the message into the appropriate dispatcher's queue
    insertUnboundedBuffer(dispatcher -> DispatcherBuffersArray[type_index], message);
}

void* dispatcherThread(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    int done_count = 0;
    while (1) {
        if(dispatcher == NULL) {
            perror("Dispatcher is NULL\n");
            exit(-1);
        }
        for (int i = 0; i < dispatcher -> TotalNumProducers; ++i) {
            if (dispatcher -> producers[i] != NULL) { // If this producer hasn't been terminated
                char* message = removeBoundedBuffer(dispatcher -> producers[i]-> ProducerBuffer); // Read news from the producer
                if (message != NULL) {
                    if (strcmp(message, "DONE") == 0) {
                        processDoneMessage(dispatcher, i, &done_count);
                        if (done_count == (dispatcher -> TotalNumProducers)) {
                            return NULL; // Exit the dispatcher thread
                        }
                    } else {
                        sendToCoEditor(dispatcher, message);
                    }
                }
                // If message is NULL, continue to the next producer's buffer without processing a message
            }
        }
    }
}

