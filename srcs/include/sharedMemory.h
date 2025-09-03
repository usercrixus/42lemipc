#pragma once

#include <stdbool.h>

bool initSharedMemory(void (*signalHandler)(int));
void destroySharedMemory();
void destroyMSGQueue();