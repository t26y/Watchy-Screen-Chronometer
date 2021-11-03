#include "battery.h"

#include <esp_adc_cal.h>

#include "Watchy.h"  // for logging macros

namespace Watchy {

esp_adc_cal_characteristics_t *getADCCharacteristics() {
  static esp_adc_cal_characteristics_t *adc_chars;  // default is nullptr
  if (!adc_chars) {
    // only initialize it if we're actually going to use it.
    adc_chars = new esp_adc_cal_characteristics_t();
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_GPIO33_CHANNEL, ADC_ATTEN_DB_11);
    esp_adc_cal_value_t cal = esp_adc_cal_characterize(
        ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);
    if (cal == ESP_ADC_CAL_VAL_DEFAULT_VREF) {
      log_w("adc calibration is using default vref. Accuracy will suffer.");
    }
  }
  return adc_chars;
}

float getBatteryVoltage() {
  // make sure ADC is initialized before we read it.
  esp_adc_cal_characteristics_t *adcChar = getADCCharacteristics();
  // average 4 samples to reduce noise
  int raw =
      (adc1_get_raw(ADC1_GPIO33_CHANNEL) + adc1_get_raw(ADC1_GPIO33_CHANNEL) +
       adc1_get_raw(ADC1_GPIO33_CHANNEL) + adc1_get_raw(ADC1_GPIO33_CHANNEL) +
       2) /
      4;
  return esp_adc_cal_raw_to_voltage(raw, adcChar) * 2.0 / 1000.0;
}
}  // namespace Watchy