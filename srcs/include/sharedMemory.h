#pragma once

#include <stdbool.h>

bool initSharedMemory();
void destroySharedMemory();
void destroyMSGQueue();