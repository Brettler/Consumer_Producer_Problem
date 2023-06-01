# include "Queue.h"

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
    // Acquiring the mutex lock to enter the critical section
    sem_wait(&buffer -> mutexSemaphore); // Enter critical section
    buffer -> buffer[buffer -> in] = article; // Insert article
    buffer -> in = (buffer -> in + 1) % buffer -> size; // Updates the in index to point to the next free slot in the buffer.
    sem_post(&buffer -> mutexSemaphore); // Exit critical section and releasing the mutex lock.
    sem_post(&buffer -> articlesSemaphore); // Increment items
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
        char **temp = realloc(buffer -> buffer, buffer -> size * sizeof(char*));
        if (temp == NULL) {
            perror("Failed to reallocate memory for Unbounded Buffer");
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
    sem_wait(&buffer -> articlesSemaphore); // Decrement the number of items
    sem_wait(&buffer -> mutexSemaphore); // Enter critical section
    char* article = buffer -> buffer[buffer->out];
    buffer -> buffer[buffer -> out] = NULL; // Clear the slot
    buffer -> out = (buffer -> out + 1) % buffer -> size; // Increment 'out'
    sem_post(&buffer -> mutexSemaphore); // Exit critical section
    sem_post(&buffer -> slotsSemaphore); // Increment the number of free slots

    return article;
}

char* removeUnboundedBuffer(UnboundedBuffer* buffer) {
    sem_wait(&buffer -> articlesSemaphore); // Decrement the number of items
    sem_wait(&buffer -> mutexSemaphore); // Enter critical section
    char* article = buffer -> buffer[buffer -> out];
    buffer->buffer[buffer -> out] = NULL; // Clear the slot
    buffer -> out += 1; // Increment 'out'
    sem_post(&buffer -> mutexSemaphore); // Exit critical section
    return article;
}

// Destructor for BoundedBuffer
void destructorBoundedBuffer(BoundedBuffer* buffer) {
    if (buffer == NULL) {
        return;
    }
    // Destroy semaphores
    sem_destroy(&buffer -> mutexSemaphore);
    sem_destroy(&buffer -> slotsSemaphore);
    sem_destroy(&buffer -> articlesSemaphore);
    // Free the buffer array within the BoundedBuffer structure
    free(buffer -> buffer);
    // Free the BoundedBuffer structure itself
    free(buffer);
}

// Destructor for UnboundedBuffer
void destructorUnboundedBuffer(UnboundedBuffer* buffer) {
    if (buffer == NULL) {
        return;
    }
    // Destroy semaphores
    sem_destroy(&buffer -> mutexSemaphore);
    sem_destroy(&buffer -> articlesSemaphore);
    // Free the buffer array within the UnboundedBuffer structure
    free(buffer -> buffer);
    // Free the UnboundedBuffer structure itself
    free(buffer);
}