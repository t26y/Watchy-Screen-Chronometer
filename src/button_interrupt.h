#pragma once

#include <Arduino.h>

typedef enum { menu_btn = 0, back_btn, up_btn, down_btn } ButtonIndex;

/* Attach interrupt to pin, which will control bit <index> in the result of
 * buttonGetPressMask */
void buttonSetup(int pin, ButtonIndex index);
int8_t buttonGet();
inline bool buttonWasPressed(const uint8_t pressMask, const ButtonIndex index) {
  return (pressMask & (1 << index)) != 0;
}
