#include "core/live_log.h"
#include "app/app_context.h"
#include "app/mode_factory.h"
#include "core/string_utils.h"
#include "hw/sensor_service.h"
#include "hw/hvac/hvac_registry.h"
#include "net/mqtt_service.h"
#include "ota/ota_manager.h"
#include <LittleFS.h>

extern "C" {
#include <user_interface.h>
}

AppContext ctx;

void AppContext::begin() {
    chipId_  = String(ESP.getChipId(), HEX);
    mac_     = WiFi.macAddress();
    slaveId_ = str::slaveIdFromMac(mac_);
    appLog.printf("[CTX] chipId=%s mac=%s slaveId=%s\n",
                  chipId_.c_str(), mac_.c_str(), slaveId_.c_str());

    configStore.begin(true);   // 裸分区（全擦后）在启动时自愈格式化，别等保存时才发现
    configStore.load();
    slaves.load();
    hvacState.loadFrom(configStore.cfg);

    ir.begin();
    hvacRegistry.begin(ir);
    sensors.begin();
    mqtt.begin();
    ota.begin();
}

void AppContext::loop() {
    appLog.tick();
    leds.loop();
    button.loop();
    pairing.loop();
    configStore.saveIfDue();
    ota.confirmIfReady();

    if (mode) mode->loop();

    yield();
}

void AppContext::changeMode(DeviceModeId id) {
    if (mode) {
        mode = nullptr;
    }
    modeId_ = id;
    auto owned = createMode(id);
    mode = owned.release();
    if (mode) {
        appLog.printf("[CTX] enter mode: %s\n", mode->name());
        mode->onEnter();
    } else {
        appLog.println(F("[CTX] createMode returned nullptr"));
    }
}

void AppContext::reboot(uint32_t delayMs) {
    appLog.printf("[CTX] reboot in %u ms\n", (unsigned)delayMs);
    delay(delayMs);
    ESP.restart();
}

DeviceModeId AppContext::decideBootMode() {
    Config& cfg = configStore.cfg;
    bool hasSTA = strlen(cfg.sta_ssid) > 0;

    if (cfg.force_mode == FORCE_MODE_AP) {
        appLog.println(F("[BOOT] force_mode=AP"));
        return DeviceModeId::ApMaster;
    }

    if (cfg.force_mode == FORCE_MODE_HOME) {
        if (!hasSTA) {
            appLog.println(F("[BOOT] force_mode=HOME without STA, revert AUTO"));
            cfg.force_mode = FORCE_MODE_AUTO;
            configStore.save();
            return DeviceModeId::ApMaster;
        }
        appLog.println(F("[BOOT] force_mode=HOME, connecting STA"));
        leds.setAll(true, false, false);
        if (network.connectSTABlocking(cfg.sta_ssid, cfg.sta_pass)) {
            configStore.scheduleSave();   // 清空 last_sta_error 需落盘
            return DeviceModeId::StaHome;
        }
        appLog.println(F("[BOOT] HOME STA failed"));
        configStore.save();               // 持久化失败原因
        if (ota.rollbackIfPending("force_mode=home, STA failed")) {
            return DeviceModeId::ApMaster;
        }
        leds.blink(PIN_LED_RED, 3, 80, 80);
        cfg.force_mode = FORCE_MODE_AUTO;
        configStore.save();
        return DeviceModeId::ApMaster;
    }

    if (cfg.force_mode == FORCE_MODE_SLAVE) {
        appLog.println(F("[BOOT] force_mode=SLAVE, scanning for master"));
        leds.setAll(false, false, true);
        String foundSsid;
        uint8_t bssid[6] = {0};
        int32_t channel = 0;
        if (network.scanForMaster(getApSsid(), foundSsid, bssid, channel)) {
            WiFi.mode(WIFI_STA);
            WiFi.begin(foundSsid.c_str(), getApPass(), channel, bssid);
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 40) {
                leds.setBlue(attempts % 2 == 0);
                leds.setYellow(attempts % 2 != 0);
                delay(500);
                attempts++;
            }
            if (WiFi.status() == WL_CONNECTED) {
                return DeviceModeId::StaSlave;
            }
            appLog.println(F("[BOOT] SLAVE connect failed"));
        } else {
            appLog.println(F("[BOOT] SLAVE no master found"));
        }
        if (ota.rollbackIfPending("force_mode=slave, no master")) {
            return DeviceModeId::ApMaster;
        }
        leds.blink(PIN_LED_RED, 3, 80, 80);
        cfg.force_mode = FORCE_MODE_AUTO;
        configStore.save();
        return DeviceModeId::ApMaster;
    }

    if (hasSTA) {
        appLog.println(F("[BOOT] AUTO, trying STA Home"));
        leds.setAll(true, false, false);
        if (network.connectSTABlocking(cfg.sta_ssid, cfg.sta_pass)) {
            configStore.scheduleSave();   // 清空 last_sta_error 需落盘
            return DeviceModeId::StaHome;
        }
        appLog.println(F("[BOOT] STA failed, trying slave scan"));
        configStore.scheduleSave();       // 持久化失败原因，AP 配网页显示
    }

    appLog.println(F("[BOOT] scanning for master"));
    leds.setAll(false, false, true);
    String foundSsid;
    uint8_t bssid[6] = {0};
    int32_t channel = 0;
    if (network.scanForMaster(getApSsid(), foundSsid, bssid, channel)) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(foundSsid.c_str(), getApPass(), channel, bssid);
        appLog.printf("[BOOT] slave connecting to %s...\n", foundSsid.c_str());

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 40) {
            leds.setBlue(attempts % 2 == 0);
            leds.setYellow(attempts % 2 != 0);
            delay(500);
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            appLog.printf("[BOOT] slave connected, IP=%s\n", WiFi.localIP().toString().c_str());
            return DeviceModeId::StaSlave;
        }
        appLog.println(F("[BOOT] slave connect failed"));
    }

    appLog.println(F("[BOOT] fallback to AP Master"));
    return DeviceModeId::ApMaster;
}

const char* AppContext::modeName() const {
    return modeString(modeId_);
}

const char* modeString(DeviceModeId id) {
    switch (id) {
        case DeviceModeId::ApMaster: return "ap";
        case DeviceModeId::StaSlave: return "slave";
        case DeviceModeId::StaHome:  return "sta";
    }
    return "unknown";
}

const char* getApSsid() {
    if (strlen(configStore.cfg.ap_ssid) > 0) return configStore.cfg.ap_ssid;
    return AP_DEFAULT_SSID;
}

const char* getApPass() {
    if (strlen(configStore.cfg.ap_pass) > 0) return configStore.cfg.ap_pass;
    return AP_DEFAULT_PASS;
}

void HvacState::applyFrom(const hvac::Command& cmd) {
    vendor = cmd.vendor;
    model = cmd.model;
    power  = cmd.power;
    mode   = cmd.mode;
    temp   = cmd.temp;
    fan    = cmd.fan;
}

void HvacState::persistTo(Config& cfg) const {
    str::copyTo(cfg.last_vendor, sizeof(cfg.last_vendor), vendor);
    str::copyTo(cfg.last_mode,   sizeof(cfg.last_mode),   mode);
    cfg.last_temp  = (uint8_t)temp;
    str::copyTo(cfg.last_fan,    sizeof(cfg.last_fan),    fan);
    cfg.last_power = power;
    cfg.last_model = model;
}

void HvacState::loadFrom(const Config& cfg) {
    if (strlen(cfg.last_vendor) > 0) vendor = cfg.last_vendor;
    model = cfg.last_model;
    power = cfg.last_power;
    if (strlen(cfg.last_mode) > 0) mode = cfg.last_mode;
    temp = constrain((int)cfg.last_temp, HVAC_MIN_TEMP_DEFAULT, HVAC_MAX_TEMP_DEFAULT);
    if (strlen(cfg.last_fan) > 0) fan = cfg.last_fan;
}

void SlaveRegistry::load() {
    for (size_t i = 0; i < CAPACITY; i++) slaves_[i] = SlaveInfo{};
    if (!ensureFSMounted(false)) return;

    File f = LittleFS.open(FS_SLAVES_TXT, "r");
    if (!f) return;

    size_t slot = 0;
    while (f.available() && slot < CAPACITY) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        int p1 = line.indexOf('|');
        int p2 = line.indexOf('|', p1 + 1);
        int p3 = line.indexOf('|', p2 + 1);
        if (p1 != 17 || p2 < 0 || p3 < 0) continue;

        String mac   = line.substring(0, p1);
        String name  = line.substring(p1 + 1, p2);
        String icon  = line.substring(p2 + 1, p3);
        String floor = line.substring(p3 + 1);
        if (!str::isValidMacString(mac)) continue;
        name  = str::sanitizeMetadata(name,  sizeof(slaves_[slot].name)  - 1);
        icon  = str::sanitizeIconKey(icon);
        floor = str::sanitizeMetadata(floor, sizeof(slaves_[slot].floor) - 1);

        str::copyTo(slaves_[slot].mac,   sizeof(slaves_[slot].mac),   mac);
        str::copyTo(slaves_[slot].name,  sizeof(slaves_[slot].name),  name);
        str::copyTo(slaves_[slot].icon,  sizeof(slaves_[slot].icon),  icon);
        str::copyTo(slaves_[slot].floor, sizeof(slaves_[slot].floor), floor);
        slaves_[slot].ip       = 0;
        slaves_[slot].lastSeen = 0;
        slot++;
    }
    f.close();
    appLog.printf("[SLAVES] loaded %u paired\n", (unsigned)slot);
}

void SlaveRegistry::save() {
    if (!ensureFSMounted(true)) return;

    File f = LittleFS.open("/slaves.tmp", "w");
    if (!f) {
        appLog.println(F("[SLAVES] save failed"));
        return;
    }
    for (size_t i = 0; i < CAPACITY; i++) {
        if (slaves_[i].mac[0] == '\0') continue;
        String name  = str::sanitizeMetadata(String(slaves_[i].name),  sizeof(slaves_[i].name)  - 1);
        String icon  = str::sanitizeIconKey(String(slaves_[i].icon));
        String floor = str::sanitizeMetadata(String(slaves_[i].floor), sizeof(slaves_[i].floor) - 1);
        f.printf("%s|%s|%s|%s\n", slaves_[i].mac, name.c_str(), icon.c_str(), floor.c_str());
    }
    f.close();

    LittleFS.remove(FS_SLAVES_BAK);
    bool hadOld = LittleFS.exists(FS_SLAVES_TXT);
    if (hadOld && !LittleFS.rename(FS_SLAVES_TXT, FS_SLAVES_BAK)) {
        appLog.println(F("[SLAVES] backup rename failed"));
        LittleFS.remove("/slaves.tmp");
        return;
    }
    if (!LittleFS.rename("/slaves.tmp", FS_SLAVES_TXT)) {
        appLog.println(F("[SLAVES] commit rename failed"));
        if (hadOld) LittleFS.rename(FS_SLAVES_BAK, FS_SLAVES_TXT);
        return;
    }
    if (hadOld) LittleFS.remove(FS_SLAVES_BAK);
    appLog.println(F("[SLAVES] saved"));
}

void SlaveRegistry::clear() {
    for (size_t i = 0; i < CAPACITY; i++) slaves_[i] = SlaveInfo{};
}

int SlaveRegistry::findByMac(const char* mac) const {
    for (size_t i = 0; i < CAPACITY; i++) {
        if (slaves_[i].mac[0] != '\0' && strcmp(slaves_[i].mac, mac) == 0) return (int)i;
    }
    return -1;
}

int SlaveRegistry::findById(const String& id) const {
    for (size_t i = 0; i < CAPACITY; i++) {
        if (slaves_[i].mac[0] != '\0' && str::slaveIdFromMac(slaves_[i].mac) == id) return (int)i;
    }
    return -1;
}

int SlaveRegistry::findFree() const {
    for (size_t i = 0; i < CAPACITY; i++) {
        if (slaves_[i].mac[0] == '\0') return (int)i;
    }
    return -1;
}

void SlaveRegistry::clearSlot(size_t idx) {
    if (idx < CAPACITY) slaves_[idx] = SlaveInfo{};
}

void SlaveRegistry::pruneOffline() {
    uint32_t now = millis();
    for (size_t i = 0; i < CAPACITY; i++) {
        if (slaves_[i].mac[0] != '\0' && slaves_[i].lastSeen != 0 &&
            (uint32_t)(now - slaves_[i].lastSeen) >= SLAVE_OFFLINE_TIMEOUT_MS) {
            appLog.printf("[SLAVES] prune offline %s\n", slaves_[i].mac);
            slaves_[i] = SlaveInfo{};
        }
    }
}

void SlaveRegistry::refreshFromApStations() {
    struct station_info *stat_info = wifi_softap_get_station_info();
    while (stat_info) {
        char mac[18];
        snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                 stat_info->bssid[0], stat_info->bssid[1], stat_info->bssid[2],
                 stat_info->bssid[3], stat_info->bssid[4], stat_info->bssid[5]);
        int slot = findByMac(mac);
        if (slot >= 0) {
            slaves_[slot].ip       = stat_info->ip.addr;
            slaves_[slot].lastSeen = millis();
        }
        stat_info = STAILQ_NEXT(stat_info, next);
    }
    wifi_softap_free_station_info();
}

uint32_t PairingWindow::remainingMs() const {
    if (!active_) return 0;
    uint32_t now = millis();
    return (until_ > now) ? (until_ - now) : 0;
}

void PairingWindow::start() {
    active_ = true;
    until_  = millis() + PAIRING_DURATION_MS;
    appLog.println(F("[PAIR] window opened"));
}

void PairingWindow::stop() {
    active_ = false;
    appLog.println(F("[PAIR] window closed"));
}

void PairingWindow::loop() {
    if (active_ && (long)(millis() - until_) >= 0) {
        active_ = false;
        appLog.println(F("[PAIR] window expired"));
    }
}
