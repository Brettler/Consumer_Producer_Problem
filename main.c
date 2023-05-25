#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define NUM_TYPES 3

char* types[NUM_TYPES] = {"Sports", "News", "Weather"};


//typedef struct {
//    int id;
//    BoundedBuffer* buffer;
//} Producer;



// This struct holds the configuration for a single producer.
typedef struct {
    int num_articles;
    int queue_size;
} Producer;

// This struct holds the entire configuration.
typedef struct {
    Producer* ArrayProducers; // Array of producer configurations
    int num_producers; // Number of producers ( The last ID of the last producer is the number of producers)
    int QueueLengthCoEditor; // Size of the co-editor queue
} Config;

typedef struct {
    Producer** producers;
    int num_producers;
} Dispatcher;


typedef struct {
    // Your code here
} BoundedBuffer;

Config* parse_config(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // The configuration for each producer will be stored in a dynamically
    // allocated array, which we'll resize as needed. Start with space for
    // one producer.
    int capacity = 1;
    Producer* ArrayProducers = malloc(capacity * sizeof(Producer));

    int num_producers = 0;
    int num_articles, queue_size;
    // Each line correspond to
    while (fscanf(file, "%d\n%d\n%d\n", &num_producers, &num_articles, &queue_size) == 3) {
        // If the array is full, double its size
        if (num_producers > capacity) {
            capacity *= 2;
            ArrayProducers = realloc(ArrayProducers, capacity * sizeof(Producer));
        }

        // Store this producer's attributes.
        // we use 'num_producers-1' because the ID of the producer starts from 1 and the array starts from zero.
        ArrayProducers[num_producers-1].num_articles = num_articles;
        ArrayProducers[num_producers-1].queue_size = queue_size;
    }

    // The last number read is the co-editor queue size, not the ID of a producer
    int QueueLengthCoEditor = num_producers;

    fclose(file);

    Config* config = malloc(sizeof(Config));
    config->ArrayProducers = ArrayProducers;
    config->num_producers = num_producers - 1;  // subtract one because the last number was the co-editor queue size
    config->QueueLengthCoEditor = QueueLengthCoEditor;

    return config;
}

BoundedBuffer* create_bounded_buffer(int size) {
    BoundedBuffer* buffer = malloc(sizeof(BoundedBuffer));
    // Initialize your bounded buffer here
    return buffer;
}

void insert(BoundedBuffer* buffer, char* s) {
    // Insert s into buffer
}

char* remove(BoundedBuffer* buffer) {
    // Remove and return the first object from buffer
    return NULL;
}

void* producer_thread(void* arg) {
    Producer* producer = (Producer*)arg;
    for (int j = 0; j < 10; ++j) {
        for (int i = 0; i < NUM_TYPES; ++i) {
            char* message = malloc(256);
            sprintf(message, "producer %d %s %d", producer->id, types[i], j);
            insert(producer->buffer, message);
        }
    }
    insert(producer->buffer, "DONE");
    return NULL;
}

void* dispatcher_thread(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    while (1) {
        for (int i = 0; i < dispatcher->num_producers; ++i) {
            char* message = remove(dispatcher->producers[i]->buffer);
            if (message != NULL) {
                if (strcmp(message, "DONE") == 0) {
                    // Handle "DONE" message
                } else {
                    // Handle normal message
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <config file>\n", argv[0]);
        return 1;
    }

    Config* config = parse_config(argv[1]);
    if (config == NULL) {
        return 1;
    }

    int num_producers = 5;
    Producer** producers = malloc(num_producers * sizeof(Producer*));
    for (int i = 0; i < num_producers; ++i) {
        producers[i] = malloc(sizeof(Producer));
        producers[i]->id = i;
        producers[i]->buffer = create_bounded_buffer(100);
    }

    Dispatcher* dispatcher = malloc(sizeof(Dispatcher));
    dispatcher->producers = producers;
    dispatcher->num_producers = num_producers;

    pthread_t* producer_threads = malloc(num_producers * sizeof(pthread_t));
    for (int i = 0; i < num_producers; ++i) {
        pthread_create(&producer_threads[i], NULL, producer_thread, producers[i]);
    }

    pthread_t dispatcher_thread_id;
    pthread_create(&dispatcher_thread_id, NULL, dispatcher_thread, dispatcher);

    for (int i = 0; i < num_producers; ++i) {
        pthread_join(producer_threads[i], NULL);
    }

    pthread_join(dispatcher_thread_id, NULL);

    // Clean up

    return 0;
}
