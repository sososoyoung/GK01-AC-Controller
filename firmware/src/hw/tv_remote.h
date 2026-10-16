#pragma once
#include <Arduino.h>
namespace tv {
bool isXiaomiKey(const String& key);
bool sendXiaomi(const String& key);
}
