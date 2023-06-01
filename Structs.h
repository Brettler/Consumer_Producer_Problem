//
// Created by liadb on 01/06/2023.
//

#pragma once
#ifndef TASK3_STRUCTS_H
#define TASK3_STRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#define NUM_ARTICLES_TYPES 3
extern char* types[NUM_ARTICLES_TYPES];

typedef struct {
    char **buffer;
    int size;
    int in; // Next place to insert
    int out; // Next place to remove
    sem_t mutexSemaphore; // Binary semaphore (basic lock)
    sem_t slotsSemaphore; // Counting semaphore (counting free slots)
    sem_t articlesSemaphore; // Counting semaphore (counting articles)
} BoundedBuffer;

typedef struct {
    char **buffer;
    int size;
    int in; // Next place to insert
    int out; // Next place to remove
    sem_t mutexSemaphore; // Binary semaphore (basic lock)
    sem_t slotsSemaphore; // Counting semaphore (counting free slots)
    sem_t articlesSemaphore; // Counting semaphore (counting articles)
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
    int TotalNumProducers;
} Manager;

typedef struct {
    UnboundedBuffer* dispatcherBuffer;
    BoundedBuffer* SharedBuffer;
} CoEditor;

#include "Producer.h"
#include "ProcessConfig.h"
#include "Queue.h"
#include "Dispatcher.h"
#include "CoEditor.h"
#include "manager.h"
#include "initThreads.h"
#include "initStructsObjects.h"
#include "FreeResource.h"
#endif //TASK3_STRUCTS_H
