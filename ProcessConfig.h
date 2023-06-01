#pragma once
#ifndef TASK3_PROCESSCONFIG_H
#define TASK3_PROCESSCONFIG_H
#include "Structs.h"
Producer* reallocateProducers(Producer* OldArray, int capacity);
Config* processConfig(const char* filename);
void cleanConfig(Config* config);
#endif //TASK3_PROCESSCONFIG_H
