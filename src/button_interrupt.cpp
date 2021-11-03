#include "button_interrupt.h"

#include <freertos/queue.h>

#include "Events.h"

#ifndef BUTTON_DEBOUNCE_US
#define BUTTON_DEBOUNCE_US 150000
#endif

static void IRAM_ATTR ISR_Send(const Watchy_Event::Event e) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendToBackFromISR(Watchy_Event::q, (const void *)&e,
                          &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

// just use one timer for all the buttons
static uint64_t lastIntTime = 0;
static int bounces = 0;

void IRAM_ATTR ISR_MenuButtonPress() {
  uint64_t t = micros() - lastIntTime;
  lastIntTime += t;
  if (t < BUTTON_DEBOUNCE_US) {
    bounces++;
    return;
  }
  ISR_Send(Watchy_Event::Event{
    .id = Watchy_Event::MENU_BTN_DOWN,
    .micros = lastIntTime,
    {.bounces = bounces},
    });
}

void IRAM_ATTR ISR_BackButtonPress() {
  uint64_t t = micros() - lastIntTime;
  lastIntTime += t;
  if (t < BUTTON_DEBOUNCE_US) {
    bounces++;
    return;
  }
  ISR_Send(Watchy_Event::Event{
    .id = Watchy_Event::BACK_BTN_DOWN,
    .micros = lastIntTime,
    {.bounces = bounces},
    });
}

void IRAM_ATTR ISR_UpButtonPress() {
  uint64_t t = micros() - lastIntTime;
  lastIntTime += t;
  if (t < BUTTON_DEBOUNCE_US) {
    bounces++;
    return;
  }
  ISR_Send(Watchy_Event::Event{
    .id = Watchy_Event::UP_BTN_DOWN,
    .micros = lastIntTime,
    {.bounces = bounces},
    });
}

void IRAM_ATTR ISR_DownButtonPress() {
  int64_t t = micros() - lastIntTime;
  lastIntTime += t;
  if (t < BUTTON_DEBOUNCE_US) {
    bounces++;
    return;
  }
  ISR_Send(Watchy_Event::Event{
    .id = Watchy_Event::DOWN_BTN_DOWN,
    .micros = lastIntTime,
    {.bounces = bounces},
    });
}

void buttonSetup(int pin, ButtonIndex index) {
  pinMode(pin, INPUT_PULLUP);
  switch (index) {
    case menu_btn:
      attachInterrupt(pin, ISR_MenuButtonPress, GPIO_INTR_NEGEDGE);
      break;
    case back_btn:
      attachInterrupt(pin, ISR_BackButtonPress, GPIO_INTR_NEGEDGE);
      break;
    case up_btn:
      attachInterrupt(pin, ISR_UpButtonPress, GPIO_INTR_NEGEDGE);
      break;
    case down_btn:
      attachInterrupt(pin, ISR_DownButtonPress, GPIO_INTR_NEGEDGE);
      break;
    default:
      break;
  }
}
