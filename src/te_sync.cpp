#include <Arduino.h>

#include "te_sync.h"

static uint32_t teTimeoutCount; // How many times the TE pulse wait has timed out

static volatile uint32_t tePulseCount = 0;

static constexpr int TE_PIN = 18; // Per the Waveshare wiki, this is the pin for the Tearing Effect signal

static void IRAM_ATTR onTePulse()
{
  tePulseCount++;
}

void teSyncBegin()
{
  pinMode(TE_PIN, INPUT);
  attachInterrupt(TE_PIN, onTePulse, RISING);
}

void waitForTePulse()
{
  uint32_t startPulseCount = tePulseCount;
  unsigned long startTime = millis();
  while (tePulseCount == startPulseCount)
  {
    if (millis() - startTime >= TE_TIMEOUT)
    {
      teTimeoutCount++;
      break; // Timeout reached, exit the wait loop
    }
  }
}

uint32_t getTePulseCount()
{
  return tePulseCount;
}

uint32_t getTeTimeoutCount()
{
  return teTimeoutCount;
}
