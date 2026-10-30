#pragma once
#include "FreeRTOS.h"
typedef void (*TaskFunction_t)(void *);
typedef void *TaskHandle_t;
BaseType_t xTaskCreate(TaskFunction_t function, const char *name, uint32_t stack,
                       void *argument, UBaseType_t priority, TaskHandle_t *handle);
