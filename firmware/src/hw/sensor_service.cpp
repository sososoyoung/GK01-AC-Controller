#include "hw/sensor_service.h"

SensorService sensors;

void SensorService::begin() {
    initialized_ = true;

    pinMode(PIN_SENSOR_PIR, INPUT);
    pirDetected_ = (digitalRead(PIN_SENSOR_PIR) == HIGH);

    dallas_.begin();
    present_ = (dallas_.getDeviceCount() > 0);
    if (present_) {
        dallas_.setResolution(12);
        dallas_.setWaitForConversion(false);   // 非阻塞：不再在请求里死等 750ms
        dallas_.requestTemperatures();
        convPending_ = true;
        convReqMs_   = millis();
        Serial.printf("[SENSOR] DS18B20 found, count=%u\n", dallas_.getDeviceCount());
    } else {
        Serial.println(F("[SENSOR] No DS18B20 detected"));
    }
    Serial.println(F("[MODULE] sensor_service real impl loaded"));
}

void SensorService::loop() {
    if (!initialized_) begin();
    if (initialized_ && !present_) {
        dallas_.begin();
        present_ = (dallas_.getDeviceCount() > 0);
        if (present_) {
            dallas_.setResolution(12);
            dallas_.setWaitForConversion(false);
        }
    }

    uint32_t now = millis();

    // 第二阶段：转换时间已过，直接读暂存器（非阻塞）
    if (convPending_ && (uint32_t)(now - convReqMs_) >= DS18B20_CONV_TIME_MS) {
        convPending_ = false;
        if (present_) {
            float t = dallas_.getTempCByIndex(0);
            if (t > SENSOR_TEMP_INVALID && t < SENSOR_TEMP_MAX) {
                roomTempC_ = t;
            }
        }
    }

    if ((uint32_t)(now - lastReadMs_) < SENSOR_INTERVAL_MS) return;
    lastReadMs_ = now;

    pirDetected_ = (digitalRead(PIN_SENSOR_PIR) == HIGH);

    // 第一阶段：发出转换请求后立即返回（setWaitForConversion(false)）
    if (present_) {
        dallas_.requestTemperatures();
        convPending_ = true;
        convReqMs_   = now;
    }
}
