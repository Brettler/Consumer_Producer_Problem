# include "ProcessConfig.h"

Producer* reallocateProducers(Producer* OldArray, int capacity) {
    Producer* NewArray = realloc(OldArray, capacity * sizeof(Producer));
    if (NewArray == NULL) {
        perror("Failed to reallocate memory for ArrayProducers");
        exit(-1);
    }
    return NewArray;
}

Config* processConfig(const char* filename) {
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
        exit(-1);
    }
    int tempNumPrducer = 0, NumArticles, QueueLength, TotalNumProducers;

    // Each line correspond to
    do {
        // If the array is full, double its size
        if (tempNumPrducer >= capacity) {
            capacity *= 2;
            ArrayProducers = reallocateProducers(ArrayProducers, capacity);
            if (ArrayProducers == NULL) {
                exit(-1);
            }
        }
        // Store this producer's attributes.
        // we use 'num_producers-1' because the ID of the producer starts from 1 and the array starts from zero.
        // Also, producer id start from zero and not from 1.
        ArrayProducers[tempNumPrducer-1].id = tempNumPrducer-1;
        ArrayProducers[tempNumPrducer-1].NumArticles = NumArticles;
        ArrayProducers[tempNumPrducer-1].QueueLength = QueueLength;
        TotalNumProducers = tempNumPrducer;
    } while (fscanf(file, "%d\n%d\n%d\n", &tempNumPrducer, &NumArticles, &QueueLength) == 3);

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

void cleanConfig(Config* config) {
    if (config != NULL) {
        free(config -> ArrayProducers);  // Free the array of Producers
        free(config);  // Free the Config itself
    }
}