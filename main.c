#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#define NUM_ARTICLES_TYPES 3

char* types[NUM_ARTICLES_TYPES] = {"Sports", "News", "Weather"};

typedef struct {
    char **buffer;
    int size;
    int in; // Next place to insert
    int out; // Next place to remove
    sem_t mutexSemaphore; // Binary semaphore (basic lock)
    sem_t slotsSemaphore; // Counting semaphore (counting free slots)
    sem_t articlesSemaphore; // Counting semaphore (counting items)
} BoundedBuffer;

typedef struct {
    char **buffer;
    int size;
    int in; // Next place to insert
    int out; // Next place to remove
    sem_t mutexSemaphore; // Binary semaphore (basic lock)
    sem_t slotsSemaphore; // Counting semaphore (counting free slots)
    sem_t articlesSemaphore; // Counting semaphore (counting items)
} UnboundedBuffer;

// This struct holds the information for a single producer.
typedef struct {
    int id; // ID to identify each producer
    int NumArticles; // The number of articles this producer will generate
    int QueueLength; // The size of the buffer for this producer
    BoundedBuffer* ProducerBuffer; // The buffer for this producer needs to be bounded
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
    UnboundedBuffer** DispatcherBuffersArray; // The buffer for the Dispatcher needs to be unbounded
} Dispatcher;

typedef struct {
    BoundedBuffer* SharedBuffer; // Manager's buffer
    sem_t doneSemaphore; // Semaphore for the "DONE" messages
    int doneCount; // Count of "DONE" messages
} Manager;

// The coEditorThread function now takes in a structure that contains two buffers:
// one for the Dispatcher and one for the Manager. This structure is created in main before the co-editor threads are created.
typedef struct {
    UnboundedBuffer* dispatcherBuffer;
    BoundedBuffer* SharedBuffer;
} CoEditor;

char* parse_message_type(char* message);
int get_type_index(char* type);
char* removeBoundedBuffer(BoundedBuffer* buffer);
char* removeUnboundedBuffer(UnboundedBuffer* buffer);
BoundedBuffer* constructorBoundedBuffer(int size);
UnboundedBuffer* constructorUnboundedBuffer();
void* managerThread(void* arg);
void* coEditorThread(void* arg);

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
    if (ArrayProducers == NULL) {
        perror("Failed to allocate memory for ArrayProducers");
        return NULL;
    }
    int tempNumPrducer = 0;
    int NumArticles, QueueLength;
    int TotalNumProducers;
    // Each line correspond to
    while (fscanf(file, "%d\n%d\n%d\n", &tempNumPrducer, &NumArticles, &QueueLength) == 3) {

        // If the array is full, double its size
        if (tempNumPrducer >= capacity) {
            capacity *= 2;
            ArrayProducers = realloc(ArrayProducers, capacity * sizeof(Producer));
            if (ArrayProducers == NULL) {
                perror("Failed to allocate memory for ArrayProducers");
                return NULL;
            }
        }

        // Store this producer's attributes.
        // we use 'num_producers-1' because the ID of the producer starts from 1 and the array starts from zero.
        // Also, producer id start from zero and not from 1.
        ArrayProducers[tempNumPrducer-1].id = tempNumPrducer-1;
        ArrayProducers[tempNumPrducer-1].NumArticles = NumArticles;
        ArrayProducers[tempNumPrducer-1].QueueLength = QueueLength;
        TotalNumProducers = tempNumPrducer-1; // This should reflect the actual number of producers
    }

    // The last number read is the co-editor queue size, not the ID of a producer
    int QueueLengthCoEditor = tempNumPrducer;

    fclose(file);

    Config* config = malloc(sizeof(Config));
    if (config == NULL) {
        perror("Failed to allocate memory for config");
        free(ArrayProducers);
        return NULL;
    }
    config -> ArrayProducers = ArrayProducers;
    config -> TotalNumProducers =TotalNumProducers;  // subtract one because the last number was the co-editor queue size
    config -> QueueLengthCoEditor = QueueLengthCoEditor;
    return config;
}

// Constructor of Bounded Buffer.
BoundedBuffer* constructorBoundedBuffer(int size) {
    // Allocate memory for a BounderBuffer struct and store a pointer it.
    BoundedBuffer* buffer = malloc(sizeof(BoundedBuffer));
    if (buffer == NULL) {
        perror("Failed to allocate memory for BoundedBuffer");
        return NULL;
    }

    // Define the size of the buffer array within the BoundedBuffer structure (buffer is an array of char pointers)
    buffer -> buffer = malloc(size * sizeof(char*));

    // Iterate over all the cells in the buffer and init them with null value.
    for (int i = 0; i < size; ++i) {
        buffer -> buffer[i] = NULL;
    }
    buffer -> size = size;
    buffer -> in = 0;
    buffer -> out = 0;
    // 'pshared' is set to 0, the semaphore is shared between threads of the same process.
    // 'value' is set to 1 = mutex lock ->  Only one thread can "own" this lock at a time.
    // When value is 1 indicates that the lock is available,
    // while a value of 0 would indicate that the lock is not available
    sem_init(&buffer -> mutexSemaphore, 0, 1);
    // initialized to the size of the buffer (indicating that all slots are initially free).
    sem_init(&buffer -> slotsSemaphore, 0, size);
    // initialized to 0 (indicating that there are initially no items in the buffer).
    sem_init(&buffer -> articlesSemaphore, 0, 0);
    return buffer;
}

// Constructor for Unbounded Buffer
UnboundedBuffer* constructorUnboundedBuffer() {
    // Allocate memory for a UnboundedBuffer struct and store a pointer it.
    UnboundedBuffer* buffer = malloc(sizeof(UnboundedBuffer));
    if (buffer == NULL) {
        perror("Failed to allocate memory for UnboundedBuffer");
        return NULL;
    }

    // Define the size of the buffer array within the BoundedBuffer structure (buffer is an array of char pointers)
    buffer -> buffer = malloc(1 * sizeof(char*)); // Start with a size of 1
    buffer -> buffer[0] = NULL;
    buffer -> size = 1;
    buffer -> in = 0;
    buffer -> out = 0;
    // 'pshared' is set to 0, the semaphore is shared between threads of the same process.
    // 'value' is set to 1 = mutex lock ->  Only one thread can "own" this lock at a time.
    // When value is 1 indicates that the lock is available,
    // while a value of 0 would indicate that the lock is not available
    sem_init(&buffer -> mutexSemaphore, 0, 1);
    // initialized to 0 (indicating that there are initially no items in the buffer).
    sem_init(&buffer -> articlesSemaphore, 0, 0);
    return buffer;
}

/*
 * If the buffer is full, the insert function will block until a slot becomes free.
 */
void insertBoundedBuffer(BoundedBuffer* buffer, char* article) {
    // If slotsSemaphore value is more then zero it means there are free slots and we can continue. This line will decremnt the free slots,
    // Otherwise, if slotsSemaphore value is zero sem_wait will  block the calling thread. (the thread will be stuck in this line).
    sem_wait(&buffer -> slotsSemaphore); // Decrement free slots
//    printf("Before inserting in BoundedBuffer: buffer->in = %d, buffer->size = %d, inserting article: %s\n",
//           buffer->in, buffer->size, article);
    // Acquiring the mutex lock to enter the critical section
    sem_wait(&buffer -> mutexSemaphore); // Enter critical section
    buffer -> buffer[buffer -> in] = article; // Insert article
    buffer -> in = (buffer -> in + 1) % buffer -> size; // Updates the in index to point to the next free slot in the buffer.
    sem_post(&buffer -> mutexSemaphore); // Exit critical section and releasing the mutex lock.
    sem_post(&buffer -> articlesSemaphore); // Increment items
//    printf("After inserting in BoundedBuffer: buffer->in = %d\n", buffer->in);

}

/*
 * If the buffer is full, the insert function will double the space, so it will behave like infinity space.
 * We dont need to use 'slotsSemaphore' because there will be always space for Articles.
 */
void insertUnboundedBuffer(UnboundedBuffer* buffer, char* article) {
    sem_wait(&buffer -> mutexSemaphore); // Enter critical section
    if (buffer -> in == buffer -> size) { // If buffer is full
        // Resize the buffer (double the size)
        buffer -> size *= 2;
        //buffer -> buffer = realloc(buffer -> buffer, buffer -> size * sizeof(char*));
        char **temp = realloc(buffer -> buffer, buffer -> size * sizeof(char*));
        if (temp == NULL) {
            perror("Failed to reallocate memory");
            // Handle error here - you might choose to return from the function or otherwise clean up
        } else {
            buffer -> buffer = temp;
        }
    }

    buffer -> buffer[buffer -> in] = article; // Insert item
    buffer -> in += 1; // Increment 'in' (move to the next cell)

    sem_post(&buffer -> mutexSemaphore); // Exit critical section and releasing the mutex lock.
    sem_post(&buffer -> articlesSemaphore); // Increment items
}

/*
 * If the buffer is empty, the remove function will NOT block until an item becomes available.
 */
char* removeBoundedBuffer(BoundedBuffer* buffer) {
    sem_wait(&buffer->articlesSemaphore); // Decrement the number of items
    sem_wait(&buffer->mutexSemaphore); // Enter critical section
//    printf("Before removing from BoundedBuffer: buffer->out = %d, buffer->size = %d\n",
//           buffer->out, buffer->size);
    char* item = buffer->buffer[buffer->out];
    buffer->buffer[buffer->out] = NULL; // Clear the slot
    buffer->out = (buffer->out + 1) % buffer->size; // Increment 'out'

    sem_post(&buffer->mutexSemaphore); // Exit critical section
    sem_post(&buffer->slotsSemaphore); // Increment the number of free slots
//    printf("After removing from BoundedBuffer: buffer->out = %d, removed article: %s\n",
//           buffer->out, item);
    return item;
}

char* removeUnboundedBuffer(UnboundedBuffer* buffer) {
    sem_wait(&buffer->articlesSemaphore); // Decrement the number of items
    sem_wait(&buffer->mutexSemaphore); // Enter critical section

    char* item = buffer->buffer[buffer->out];
    buffer->buffer[buffer->out] = NULL; // Clear the slot
    buffer->out += 1; // Increment 'out'

    sem_post(&buffer->mutexSemaphore); // Exit critical section

    return item;
}

void* producerThread(void* arg) {
    Producer* producer = (Producer*)arg;
    int articleCounts[NUM_ARTICLES_TYPES] = {0}; // Count of articles for each type
    for (int j = 0; j < producer -> NumArticles; ++j) {
        int i = rand() % NUM_ARTICLES_TYPES; // Choose a random type
        char* message = malloc(100);
        sprintf(message, "Producer %d %s %d", producer -> id, types[i], articleCounts[i]++);
        insertBoundedBuffer(producer -> ProducerBuffer, message);
    }
    insertBoundedBuffer(producer -> ProducerBuffer, "DONE");
    return NULL;
}

void* dispatcherThread(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    // UPDATE:
    int done_count = 0;
    while (1) {
        if(dispatcher == NULL) {
            printf("Dispatcher is NULL\n");
            return NULL;
        }
        for (int i = 0; i <= dispatcher -> TotalNumProducers; ++i) {
            if (dispatcher -> producers[i] != NULL) { // If this producer hasn't been terminated
                char* message = removeBoundedBuffer(dispatcher -> producers[i]-> ProducerBuffer); // Read news from the producer
                if (message != NULL) {
                    if (strcmp(message, "DONE") == 0) {
                        // Terminate and clean up this producer
                        free(dispatcher -> producers[i] -> ProducerBuffer -> buffer); // Free the internal buffer
                        free(dispatcher -> producers[i] -> ProducerBuffer); // Free the bounded buffer struct
                        free(dispatcher -> producers[i]); // Free the producer struct
                        dispatcher -> producers[i] = NULL; // Set the pointer in the producers array to NULL
                        if (done_count == dispatcher -> TotalNumProducers) {
                            // All producers are done, send "DONE" through each dispatcher's queue and exit loop
                            for (int j = 0; j < NUM_ARTICLES_TYPES; ++j) {
                                insertUnboundedBuffer(dispatcher -> DispatcherBuffersArray[j], "DONE");
                                //printf("Inserted DONE message to the dispatcher queue %d\n", j);
                            }
                            return NULL; // Exit the dispatcher thread
                        }
                        // Increment the counter how much times we saw done. in the third time it will
                        done_count++;

                    } else {
                        // Handle normal message
                        // Parse the message to determine its type
                        char* type = parse_message_type(message);
                        int type_index = get_type_index(type);

                        // Insert the message into the appropriate dispatcher's queue
                        insertUnboundedBuffer(dispatcher -> DispatcherBuffersArray[type_index], message);

                    }
                }
                // If message is NULL, continue to the next producer's buffer without processing a message
            }
        }
    }
}

// Parse the message type
char* parse_message_type(char* message) {
    char* copy = strdup(message);
    char* token = strtok(copy, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");
    char* type = strdup(token);
    free(copy);
    return type;
}

// Get the type index
int get_type_index(char* type) {
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        if (strcmp(type, types[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void* coEditorThread(void* arg) {
    CoEditor* args = (CoEditor*)arg;
    UnboundedBuffer* dispatcherBuffer = args -> dispatcherBuffer;
    BoundedBuffer* managerBuffer = args -> SharedBuffer;

    while (1) {
        char* message = removeUnboundedBuffer(dispatcherBuffer);
        if (strcmp(message, "DONE") == 0) {
            insertBoundedBuffer(managerBuffer, "DONE"); // Forward the "DONE" message
            break; // Exit the loop
        }

        // Simulate editing by waiting for 0.1 seconds
        sleep((unsigned int) 0.01);
        insertBoundedBuffer(managerBuffer, message); // Forward the message to the manager
    }
    return NULL;
}


void* managerThread(void* arg) {
    Manager* manager = (Manager*)arg;
    while(1) {
        char* message = removeBoundedBuffer(manager -> SharedBuffer);
        if (strcmp(message, "DONE") == 0) {
            manager -> doneCount += 1; // Decrement doneCount for each "DONE" message
            if (manager -> doneCount == 3) {
                // Exit the loop when all "DONE" messages have been received
                break;
            }
            continue;
        }
        printf("%s\n", message);
    }
    //printf("DONE");
    return NULL;
}

int main(int argc, char** argv) {


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
    Producer** ArrayProducers = malloc(config -> TotalNumProducers * sizeof(Producer*));

    // Print the values in config
    for (int i = 0; i <= config -> TotalNumProducers; ++i) {
        ArrayProducers[i] = malloc(sizeof(Producer*)); // Allocate memory for each producer
        ArrayProducers[i] -> id = config -> ArrayProducers[i].id; // Assign each producer its ID from configuration
        ArrayProducers[i] -> NumArticles = config -> ArrayProducers[i].NumArticles;

        // Create a bounded buffer for each producer with the specified queue size
        // We create here in the main so the dispatcher wil have acess to it.
        ArrayProducers[i]-> ProducerBuffer = constructorBoundedBuffer(config -> ArrayProducers[i].QueueLength);
        // Print the information of the producer before creating the thread
//        printf("Producer %d: id=%d, QueueLength=%d, NumArticles=%d\n", i, ArrayProducers[i]->id,
//               config -> ArrayProducers[i].QueueLength, ArrayProducers[i] -> NumArticles);
    }

    // Create an array of BoundedBuffer pointers, one for each type of message
    UnboundedBuffer** DispatcherBuffersArray = malloc(NUM_ARTICLES_TYPES * sizeof(UnboundedBuffer*));
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        // Create a bounded buffer for each type of message with the specified queue length
        DispatcherBuffersArray[i] = constructorUnboundedBuffer();
    }

    // Create a dispatcher and assign the producers to it
    Dispatcher* dispatcher = malloc(sizeof(Dispatcher));
    dispatcher -> producers = ArrayProducers;
    dispatcher -> TotalNumProducers = config -> TotalNumProducers;
    dispatcher -> DispatcherBuffersArray = DispatcherBuffersArray;  // Assign DispatcherBuffersArray

    // Create a thread for each producer
    pthread_t* ProducerThreads = malloc(config -> TotalNumProducers * sizeof(pthread_t));
    for (int i = 0; i <= config -> TotalNumProducers; ++i) {
        // The third argument is the function to be executed by the thread
        // The fourth argument is passed as the sole argument of that function
        pthread_create(&ProducerThreads[i], NULL, producerThread, ArrayProducers[i]);
    }

    // Create a thread for the dispatcher with set the function
    pthread_t dispatcher_thread_id;
    pthread_create(&dispatcher_thread_id, NULL, dispatcherThread, dispatcher);



    // Create a shared bounded buffer for the co-editors and the manager
    BoundedBuffer* SharedBuffer = constructorBoundedBuffer(config -> QueueLengthCoEditor);

    // Create co-editor threads
    pthread_t* coEditorThreads = malloc(NUM_ARTICLES_TYPES * sizeof(pthread_t));
    CoEditor** ArrayCoEditors = malloc(NUM_ARTICLES_TYPES * sizeof(CoEditor*));

    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        ArrayCoEditors[i] = malloc(sizeof(CoEditor));
        ArrayCoEditors[i] -> dispatcherBuffer = dispatcher -> DispatcherBuffersArray[i];
        ArrayCoEditors[i] -> SharedBuffer = SharedBuffer;
        pthread_create(&coEditorThreads[i], NULL, coEditorThread, ArrayCoEditors[i]);
        //printf("Created CoEditorThread %d\n", i);

    }

    // Allocate memory for Struct manager and declare it.
    Manager* manager = malloc(sizeof(Manager));
    manager -> SharedBuffer = SharedBuffer;
    manager -> doneCount = 0;

    // Create manager thread
    pthread_t ManagerThreadID;
    pthread_create(&ManagerThreadID, NULL, managerThread, manager);


    // Join threads

           // Wait for all producer threads to finish execution
    for (int i = 0; i <= config -> TotalNumProducers; ++i) {
        pthread_join(ProducerThreads[i], NULL);
    }

    // Wait for the dispatcher thread to finish execution
    pthread_join(dispatcher_thread_id, NULL);


    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        pthread_join(coEditorThreads[i], NULL);
        free(ArrayCoEditors[i]); // Free CoEditorArgs when they're no longer needed
    }
    //printf("We are waiting to the manager thread to finish");
    pthread_join(ManagerThreadID, NULL);
    //printf("######### \n We are STOPPED waiting for the manager thread. \n ########################");




    // Clean up all dynamically allocated memory
    for (int i = 0; i <= config -> TotalNumProducers; ++i) {
        free(ArrayProducers[i]);
    }
    free(ArrayProducers);
    free(config -> ArrayProducers);
    free(config);
    for (int i = 0; i < NUM_ARTICLES_TYPES; ++i) {
        free(DispatcherBuffersArray[i] -> buffer);
        free(DispatcherBuffersArray[i]);
    }

    free(DispatcherBuffersArray);
    free(dispatcher);
    free(ProducerThreads);
    free(coEditorThreads);
    free(ArrayCoEditors);
    free(manager);
    return 0;
}
