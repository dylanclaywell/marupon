#include <Arduino.h>
#include <string.h>
#include <stdio.h>

#include "buttons.h"

static constexpr int LINE_BUFFER_SIZE = 32;
static char lineBuffer[LINE_BUFFER_SIZE];
static int lineLength = 0;

// Maps the textual representation of buttons received over serial to their corresponding enum values.
struct SerialButtonMapping
{
  const char *name;  // the text that arrives over serial, e.g. "UP"
  ButtonName button; // the enum value it stands for, e.g. BUTTON_UP
};

static constexpr SerialButtonMapping serialButtonMappings[] = {
    {"A", BUTTON_A},
    {"B", BUTTON_B},
    {"UP", BUTTON_UP},
    {"DOWN", BUTTON_DOWN}};
static constexpr int SERIAL_BUTTON_MAPPING_COUNT = sizeof(serialButtonMappings) / sizeof(serialButtonMappings[0]);

static ButtonState buttonStates[BUTTON_COUNT] = {};

static bool findButton(const char *name, ButtonName &out)
{
  for (int i = 0; i < SERIAL_BUTTON_MAPPING_COUNT; i++)
  {
    if (strcmp(name, serialButtonMappings[i].name) == 0)
    {
      out = serialButtonMappings[i].button;
      return true;
    }
  }
  return false;
}

void pollButtonsFromSerial()
{
  while (Serial.available() > 0)
  {
    char incomingByte = Serial.read();

    if (incomingByte == '\n')
    {
      lineBuffer[lineLength] = '\0'; // Turn the buffer into a proper C string.
      char buttonName[16];
      char buttonState[16];

      int fieldsParsed = sscanf(lineBuffer, "BUTTON %15s %15s", buttonName, buttonState);

      if (fieldsParsed == 2)
      {
        ButtonName button;
        if (findButton(buttonName, button))
        {
          buttonStates[button] = (strcmp(buttonState, "PRESSED") == 0) ? BUTTON_PRESSED : BUTTON_RELEASED;
        }
        else
        {
          Serial.print("Unknown button: ");
          Serial.println(buttonName);
        }
      }
      else
      {
        Serial.print("Could not parse line: ");
        Serial.println(lineBuffer);
      }

      lineLength = 0; // Start the next line from scratch.
    }
    else if (lineLength < LINE_BUFFER_SIZE - 1) // Leave room for the '\0' above.
    {
      lineBuffer[lineLength] = incomingByte;
      lineLength++;
    }
    // else: buffer is full and this isn't a newline yet -- drop the byte.
  }
}

ButtonState getButtonState(ButtonName button)
{
  return buttonStates[button];
}