#pragma once

#include <Arduino.h>

constexpr int TE_TIMEOUT = 25; // Timeout for waiting for the next TE pulse, in milliseconds. This is used to prevent indefinite blocking if a TE pulse is missed, and also a fallback for slow or missing TE signals for updating the display

void teSyncBegin();
void waitForTePulse();
uint32_t getTePulseCount();
uint32_t getTeTimeoutCount();