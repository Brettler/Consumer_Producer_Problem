#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define NUM_TYPES 3

char* types[NUM_TYPES] = {"Sports", "News", "Weather"};



typedef struct {
    char **buffer;
    int size;
    int in; // Next place to insert
    int out; // Next place to remove
    sem_t mutex; // Binary semaphore (basic lock)
    sem_t slots; // Counting semaphore (counting free slots)
    sem_t items; // Counting semaphore (counting items)
} BoundedBuffer;

// This struct holds the information for a single producer.
typedef struct {
    int id; // ID to identify each producer
    int NumArticles; // The number of articles this producer will generate
    int queue_size; // The size of the buffer for this producer
    BoundedBuffer* ProducerBuffer; // The buffer for this producer
} Producer;

// This struct holds the entire configuration.
typedef struct {
    Producer* ArrayProducers; // Array of producer configurations
    int TotalNumProducers; // Number of producers ( The last ID of the last producer is the number of producers)
    int QueueLengthCoEditor; // Size of the co-editor queue
} Config;

typedef struct {
    Producer** producers;
    int TotalNumProducers;
    BoundedBuffer** DispatcherBuffersArray; // The buffer for the Dispatcher
} Dispatcher;

char* parse_message_type(char* message);
int get_type_index(char* type);
char* pop(BoundedBuffer* buffer);
BoundedBuffer* constructorBoundedBuffer(int size);


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

    int TotalNumProducers = 0;
    int NumArticles, queue_size;
    // Each line correspond to
    while (fscanf(file, "%d\n%d\n%d\n", &TotalNumProducers, &NumArticles, &queue_size) == 3) {
        // If the array is full, double its size
        if (TotalNumProducers > capacity) {
            capacity *= 2;
            ArrayProducers = realloc(ArrayProducers, capacity * sizeof(Producer));
        }

        // Store this producer's attributes.
        // we use 'num_producers-1' because the ID of the producer starts from 1 and the array starts from zero.
        ArrayProducers[TotalNumProducers-1].NumArticles = NumArticles;
        ArrayProducers[TotalNumProducers-1].queue_size = queue_size;
    }

    // The last number read is the co-editor queue size, not the ID of a producer
    int QueueLengthCoEditor = TotalNumProducers;

    fclose(file);

    Config* config = malloc(sizeof(Config));
    config->ArrayProducers = ArrayProducers;
    config->TotalNumProducers = TotalNumProducers - 1;  // subtract one because the last number was the co-editor queue size
    config->QueueLengthCoEditor = QueueLengthCoEditor;

    return config;
}

// Constructor of Bounded Buffer.
BoundedBuffer* constructorBoundedBuffer(int size) {
    BoundedBuffer* buffer = malloc(sizeof(BoundedBuffer));
    buffer->buffer = malloc(size * sizeof(char*));
    for (int i = 0; i < size; ++i) {
        buffer->buffer[i] = NULL;
    }
    buffer->size = size;
    buffer->in = 0;
    buffer->out = 0;
    sem_init(&buffer->mutex, 0, 1);
    sem_init(&buffer->slots, 0, size);
    sem_init(&buffer->items, 0, 0);
    return buffer;
}

/*
 * If the buffer is full, the insert function will block until a slot becomes free.
 */
void insert(BoundedBuffer* buffer, char* s) {
    sem_wait(&buffer -> slots); // Decrement free slots
    sem_wait(&buffer -> mutex); // Enter critical section
    buffer -> buffer[buffer -> in] = s; // Insert item
    buffer -> in = (buffer -> in + 1) % buffer -> size; // Increment 'in'
    sem_post(&buffer -> mutex); // Exit critical section
    sem_post(&buffer -> items); // Increment items
}

/*
 * If the buffer is empty, the remove function will NOT block until an item becomes available.
 */
char* pop(BoundedBuffer* buffer) {
    if (sem_trywait(&buffer->items) != 0) { // If no items
        return NULL;
    }
    sem_wait(&buffer -> mutex); // Enter critical section
    char* item = buffer -> buffer[buffer -> out]; // Remove item
    buffer -> buffer[buffer -> out] = NULL; // Reset the buffer slot to NULL
    buffer -> out = (buffer -> out + 1) % buffer -> size; // Increment 'out'
    sem_post(&buffer -> mutex); // Exit critical section
    sem_post(&buffer -> slots); // Increment free slots
    return item;
}


void* producer_thread(void* arg) {
    Producer* producer = (Producer*)arg;
    for (int j = 0; j < producer->NumArticles; ++j) {
        for (int i = 0; i < NUM_TYPES; ++i) {
            char* message = malloc(256);
            sprintf(message, "producer %d %s %d", producer->id, types[i], j);
            insert(producer -> ProducerBuffer, message);
        }
    }
    insert(producer -> ProducerBuffer, "DONE");
    return NULL;
}

void* dispatcher_thread(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    int done_count = 0;
    while (1) {
        for (int i = 0; i < dispatcher->TotalNumProducers; ++i) {
            if (dispatcher->producers[i] != NULL) { // If this producer hasn't been terminated
                char* message = pop(dispatcher->producers[i]-> ProducerBuffer);
                if (message != NULL) {
                    if (strcmp(message, "DONE") == 0) {
                        // Handle "DONE" message
                        done_count++;
                        // Terminate and clean up this producer
                        free(dispatcher -> producers[i] -> ProducerBuffer ->buffer); // Free the internal buffer
                        free(dispatcher -> producers[i] -> ProducerBuffer); // Free the bounded buffer struct
                        free(dispatcher -> producers[i]); // Free the producer struct
                        dispatcher -> producers[i] = NULL; // Set the pointer in the producers array to NULL
                        if (done_count == dispatcher->TotalNumProducers) {
                            // All producers are done, send "DONE" through each dispatcher's queue and exit loop
                            for (int j = 0; j < NUM_TYPES; ++j) {
                                insert(dispatcher -> DispatcherBuffersArray[j], "DONE");
                            }
                            return NULL; // Exit the dispatcher thread
                        }
                    } else {
                        // Handle normal message
                        // Parse the message to determine its type
                        char* type = parse_message_type(message);
                        int type_index = get_type_index(type); // get_type_index is a function you'd need to implement
                        // Insert the message into the appropriate dispatcher's queue
                        insert(dispatcher -> DispatcherBuffersArray[type_index], message);
                    }
                }
                // If message is NULL, continue to the next producer's buffer without processing a message
            }
        }
    }
    return NULL;
}

// Parse the message type
char* parse_message_type(char* message) {
    char* copy = strdup(message); // Create a copy of the message string to avoid modifying the original
    // strtok splits the string into tokens separated by the delimiters (in this case, space " ").
    // It returns a pointer to the first token found in the string.
    char* type = strtok(copy, " "); // Skip the first word (the producer ID)
    type = strtok(NULL, " "); // Get the second word (the type)
    type = strdup(type); // Copy the type string to a new memory location
    free(copy); // Free the copy of the original message
    return type;
}

// Get the type index
int get_type_index(char* type) {
    for (int i = 0; i < NUM_TYPES; ++i) {
        // If the type matches one of the predefined types, return its index.
        if (strcmp(type, types[i]) == 0) {
            return i;
        }
    }
    return -1; // Return -1 if the type is not found (error)
}


int main(int argc, char** argv) {
    int i;

    if (argc != 2) {
        printf("Usage: %s <config file>\n", argv[0]);
        return 1;
    }

    // Parse configuration file
    Config* config = parse_config(argv[1]);
    if (config == NULL) {
        return 1;
    }

    // Create array of Producer pointers, one for each producer specified in the config file
    Producer** producers = malloc(config->TotalNumProducers * sizeof(Producer*));

    for (i = 0; i < config->TotalNumProducers; ++i) {
        producers[i] = malloc(sizeof(Producer)); // Allocate memory for each producer
        producers[i]-> id = i; // Assign each producer an ID
        // Create a bounded buffer for each producer with the specified queue size
        producers[i]-> ProducerBuffer = constructorBoundedBuffer(config -> ArrayProducers[i].queue_size);
    }

    // Create an array of BoundedBuffer pointers, one for each type of message
    BoundedBuffer** DispatcherBuffersArray = malloc(NUM_TYPES * sizeof(BoundedBuffer*));
    for (i = 0; i < NUM_TYPES; ++i) {
        // Create a bounded buffer for each type of message with the specified queue length
        DispatcherBuffersArray[i] = constructorBoundedBuffer(config -> QueueLengthCoEditor);
    }

    // Create a dispatcher and assign the producers to it
    Dispatcher* dispatcher = malloc(sizeof(Dispatcher));
    dispatcher->producers = producers;
    dispatcher-> TotalNumProducers = config -> TotalNumProducers;
    dispatcher->DispatcherBuffersArray = DispatcherBuffersArray;  // Assign DispatcherBuffersArray

    // Create a thread for each producer
    pthread_t* producer_threads = malloc(config -> TotalNumProducers * sizeof(pthread_t));
    for (i = 0; i < config -> TotalNumProducers; ++i) {
        // The third argument is the function to be executed by the thread
        // The fourth argument is passed as the sole argument of that function
        pthread_create(&producer_threads[i], NULL, producer_thread, producers[i]);
    }

    // Create a thread for the dispatcher
    pthread_t dispatcher_thread_id;
    pthread_create(&dispatcher_thread_id, NULL, dispatcher_thread, dispatcher);

    // Wait for all producer threads to finish execution
    for (i = 0; i < config -> TotalNumProducers; ++i) {
        pthread_join(producer_threads[i], NULL);
    }

    // Wait for the dispatcher thread to finish execution
    pthread_join(dispatcher_thread_id, NULL);

    // Clean up all dynamically allocated memory
    for (i = 0; i < config -> TotalNumProducers; ++i) {
        free(producers[i]);
    }
    free(producers);
    free(config -> ArrayProducers);
    free(config);
    for (i = 0; i < NUM_TYPES; ++i) {
        free(DispatcherBuffersArray[i] -> buffer);
        free(DispatcherBuffersArray[i]);
    }
    free(DispatcherBuffersArray);
    free(dispatcher);
    free(producer_threads);

    return 0;
}
