#pragma once

enum ButtonState
{
  BUTTON_RELEASED,
  BUTTON_PRESSED
};

enum ButtonName
{
  BUTTON_A,
  BUTTON_B,
  BUTTON_UP,
  BUTTON_DOWN,

  BUTTON_COUNT
};

void pollButtonsFromSerial();
ButtonState getButtonState(ButtonName button);