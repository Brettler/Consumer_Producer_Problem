# include "Producer.h"

// This function defines the operation of a Producer thread.
void* producerThread(void* arg) {

    Producer* producer = (Producer*)arg; // Cast the pointer to a Producer pointer, so we can access its properties.
    // This array stores the number of articles of each type the producer has produced.
    int articleCounts[NUM_ARTICLES_TYPES] = {0};
    // The Producer creates articles until it reaches its max articles provided in the config file.
    for (int j = 0; j < producer -> NumArticles; ++j) {
        int i = rand() % NUM_ARTICLES_TYPES; // Choose a random type for the article.
        // Allocate memory for the message that will be inserted in the buffer.
        char* message = malloc(100);
        sprintf(message, "Producer %d %s %d", producer -> id, types[i], articleCounts[i]++);
        // Insert the message into the producer's buffer.
        insertBoundedBuffer(producer -> ProducerBuffer, message);
    }
    // After all articles are produced, insert a "DONE" message to signal to the dispatcher the end of production.
    insertBoundedBuffer(producer -> ProducerBuffer, "DONE");
    return NULL;
}