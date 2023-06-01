#pragma once
#ifndef TASK3_QUEUE_H
#define TASK3_QUEUE_H
#include "Structs.h"
void insertUnboundedBuffer(UnboundedBuffer* buffer, char* article);
void insertBoundedBuffer(BoundedBuffer* buffer, char* article);
char* removeBoundedBuffer(BoundedBuffer* buffer);
char* removeUnboundedBuffer(UnboundedBuffer* buffer);
BoundedBuffer* constructorBoundedBuffer(int size);
UnboundedBuffer* constructorUnboundedBuffer();
void destructorBoundedBuffer(BoundedBuffer* buffer);
void destructorUnboundedBuffer(UnboundedBuffer* buffer);
#endif //TASK3_QUEUE_H
