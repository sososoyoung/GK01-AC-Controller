#include "core/live_log.h"
#include "web/web_server.h"
#include "web/api/web_api.h"
#include "ota/ota_manager.h"
#include "core/constants.h"
#include "app/app_context.h"
#include "rboot.h"
#include <webui.h>
#include "core/string_utils.h"
extern "C" {
#include <user_interface.h>
}

WebServer web;

namespace {
const char* CAPTIVE_PATHS[] = {
    "/generate_204",
    "/hotspot-detect.html",
    "/library/test/success.html",
    "/connecttest.txt",
    "/connecttest.htm",
    "/redirect",
    "/ncsi.txt",
    "/fwlink",
    "/wpad.dat"
};
}

void WebServer::beginAP(uint16_t port) {
    (void)port;
    server_.begin();
    appLog.println(F("[MODULE] web_server real impl loaded"));
}

void WebServer::beginSTA(uint16_t port) {
    (void)port;
    server_.begin();
}

void WebServer::loop() {
    server_.handleClient();
    // OTA 上传期间暂停 captive DNS：手机联网探测会与上传争抢单线程，拖垮吞吐
    if (captiveActive_ && !ota.isUploading()) dns_.processNextRequest();
}

void WebServer::stop() {
    server_.stop();
}

void WebServer::handleRoot() {
    const size_t total = sizeof(INDEX_HTML) - 1;
    server_.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    server_.sendHeader(F("Pragma"), F("no-cache"));
    server_.setContentLength(total);
    server_.send(200, "text/html; charset=UTF-8", "");
    size_t pos = 0;
    while (pos < total) {
        size_t chunk = min((size_t)1400, total - pos);
        server_.sendContent_P(INDEX_HTML + pos, chunk);
        pos += chunk;
        yield();
    }
    server_.sendContent("");
}

void WebServer::handleCaptiveRedirect() {
    String uri = server_.uri();
    if (uri.startsWith("/api/") || uri == "/update") {
        server_.send(404, "application/json", "{\"ok\":false,\"error\":\"not_found\"}");
        return;
    }
    if (uri == "/wpad.dat") {
        server_.send(404, "text/plain", "");
        return;
    }
    String host = server_.hostHeader();
    if (host.indexOf(String(AP_IP_STR)) >= 0) {
        handleRoot();
        return;
    }
    String target = (uri == "/connecttest.txt")
                    ? String("http://logout.net")
                    : String("http://") + AP_IP_STR + "/";
    server_.sendHeader(F("Location"), target, true);
    server_.send(302, "text/plain", "");
    server_.client().stop();
}

namespace {
void logResponse(int code, const String& body) {
    web.raw().sendHeader(F("Cache-Control"), F("no-store"));
    web.raw().sendHeader(F("Pragma"), F("no-cache"));
    web.raw().send(code, "application/json", body);
}
bool logAuthorized() {
    if (!web.raw().authenticate("admin", getApPass())) {
        logResponse(403, "{\"ok\":false,\"error\":\"auth_failed\"}"); return false;
    }
    if (ota.isUploading()) {
        appLog.clear();
        logResponse(503, "{\"ok\":false,\"error\":\"ota_busy\"}"); return false;
    }
    return true;
}
bool logUint(const String& value, uint32_t& out) {
    if (!value.length() || value.length() > 10) return false;
    uint64_t number = 0;
    for (size_t i = 0; i < value.length(); ++i) {
        if (value[i] < '0' || value[i] > '9') return false;
        number = number * 10 + (value[i] - '0');
        if (number > UINT32_MAX) return false;
    }
    out = uint32_t(number); return true;
}
}

void WebServer::registerApiRoutes() {
    api::registerAllRoutes();
    server_.on("/api/logs/start", HTTP_POST, []() {
        if (!logAuthorized()) return;
        uint32_t id = os_random();
        if (!id || id == appLog.queue.session()) id = appLog.queue.session() + 1;
        if (!id) id = 1;
        uint32_t cursor = appLog.start(id);
        logResponse(200, String("{\"ok\":true,\"session\":") + id +
                    ",\"cursor\":" + cursor + ",\"capacity\":20,\"lease_ms\":7000}");
    });
    server_.on("/api/logs", HTTP_GET, []() {
        if (!logAuthorized()) return;
        uint32_t session, after;
        if (!logUint(web.raw().arg("session"), session) || !logUint(web.raw().arg("after"), after)) {
            logResponse(400, "{\"ok\":false,\"error\":\"bad_cursor\"}"); return;
        }
        if (!appLog.queue.renew(session, millis())) {
            logResponse(409, "{\"ok\":false,\"error\":\"session_expired\"}"); return;
        }
        if (int32_t(after - appLog.queue.cursor()) > 0) {
            logResponse(400, "{\"ok\":false,\"error\":\"bad_cursor\"}"); return;
        }
        uint32_t dropped = appLog.queue.acknowledge(after), next = after;
        String json = "{\"ok\":true,\"entries\":[";
        constexpr size_t RESPONSE_LIMIT = 2400;
        if (!json.reserve(RESPONSE_LIMIT)) {
            logResponse(503, "{\"ok\":false,\"error\":\"low_memory\"}"); return;
        }
        size_t count = min(appLog.queue.size(), size_t(10));
        for (size_t i = 0; i < count; ++i) {
            const auto& entry = appLog.queue.at(i);
            String item = String("{\"seq\":") + entry.seq + ",\"uptime\":" + entry.uptime +
                    ",\"text\":\"" + str::jsonEscape(String(entry.text)) + "\"}";
            // JSON control escaping can expand one byte to six. Never grow
            // the response beyond its reserved budget, even for such lines.
            if (json.length() + item.length() + 80 > RESPONSE_LIMIT) break;
            if (i) json += ',';
            json += item;
            next = entry.seq;
        }
        json += String("],\"next\":") + next + ",\"dropped\":" + dropped + "}";
        logResponse(200, json);
    });
    server_.on("/api/logs/stop", HTTP_POST, []() {
        if (!logAuthorized()) return;
        uint32_t session;
        if (!logUint(web.raw().arg("session"), session)) {
            logResponse(400, "{\"ok\":false,\"error\":\"bad_session\"}"); return;
        }
        if (!appLog.queue.stop(session)) {
            logResponse(409, "{\"ok\":false,\"error\":\"session_expired\"}"); return;
        }
        appLog.clear();
        logResponse(200, "{\"ok\":true}");
    });
    server_.on("/", HTTP_GET, [this]() { this->handleRoot(); });

#if IRAC_RBOOT_OTA
    // Validate credentials before the browser sends the firmware body.
    server_.on("/api/ota/auth", HTTP_GET, [this]() {
        if (!server_.authenticate("admin", getApPass())) {
            server_.send(403, "application/json", "{\"ok\":false,\"error\":\"auth_failed\"}");
            return;
        }
        server_.send(200, "application/json", "{\"ok\":true}");
    });
    server_.on("/update", HTTP_POST,
        [this]() {
            if (!server_.authenticate("admin", getApPass())) {
                server_.send(403, "application/json", "{\"ok\":false,\"error\":\"auth_failed\"}");
                return;
            }
            if (!ota.lastUploadOk() || ota.lastUploadSize() == 0) {
                String msg = "OTA write failed";
                if (ota.lastRejectReason()[0]) { msg += ": "; msg += ota.lastRejectReason(); }
                server_.send(500, "text/plain", msg);
                return;
            }
            OtaManager::ManifestInfo info{};
            char reason[96] = "";
            uint8_t target = rboot_get_target_rom();
            uint32_t addr  = rboot_get_rom_address(target);
            if (!ota.verifyInFlash(addr, ota.lastUploadSize(), target, info, reason, sizeof(reason))) {
                String msg = "OTA image verify failed";
                if (reason[0]) { msg += ": "; msg += reason; }
                server_.send(400, "text/plain", msg);
                return;
            }
            appLog.printf("[OTA] verified %u bytes, version=%s crc=0x%08X\n",
                          (unsigned)info.imageSize, info.version, info.imageCrc32);
            server_.send(200, "text/plain", "OK, rebooting");
            ota.switchToTargetRom();
            delay(100);
            ESP.restart();
        },
        []() {
            HTTPUpload& upload = web.raw().upload();
            // START returning does not stop the parser's WRITE/END callbacks.
            // Reject every unauthenticated callback, including retries after a
            // previous upload, before it can reach the flash writer.
            if (!web.raw().authenticate("admin", getApPass())) {
                ota.onUploadAbort();
                return;
            }
            if (upload.status == UPLOAD_FILE_START) {
                appLog.clear();
                ota.onUploadStart();
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                ota.onUploadChunk(upload.buf, upload.currentSize);
            } else if (upload.status == UPLOAD_FILE_END) {
                ota.onUploadEnd();
            } else if (upload.status == UPLOAD_FILE_ABORTED) {
                ota.onUploadAbort();
            }
        });

    server_.on("/api/ota/status", HTTP_GET, []() {
        char buf[256];
        uint8_t cur = rboot_get_current_rom();
        uint8_t target = rboot_get_target_rom();
        rboot_config conf = rboot_get_config();
        snprintf(buf, sizeof(buf),
                 "{\"ok\":true,\"current_rom\":%u,\"target_rom\":%u,"
                 "\"rom0_addr\":\"0x%X\",\"rom1_addr\":\"0x%X\","
                 "\"target_addr\":\"0x%X\",\"ota_pending\":%s,"
                 "\"slot_max\":%u,\"sketch_size\":%u,\"free_space\":%u}",
                 cur, target, conf.roms[0], conf.roms[1],
                 rboot_get_rom_address(target),
                 ota.isPending() ? "true" : "false",
                 0xFE000u,
                 (unsigned)ESP.getSketchSize(),
                 (unsigned)ESP.getFreeSketchSpace());
        web.raw().send(200, "application/json", buf);
    });
#else
    server_.on("/update", HTTP_POST, []() {
        web.raw().send(503, "text/plain", "rboot OTA disabled in factory firmware");
    }, []() {
        HTTPUpload& upload = web.raw().upload();
        (void)upload;
    });
    server_.on("/api/ota/status", HTTP_GET, []() {
        web.raw().send(200, "application/json",
                       "{\"ok\":true,\"ota_supported\":false,\"mode\":\"factory\"}");
    });
#endif
}

void WebServer::startCaptivePortal() {
    if (captiveActive_) return;
    captiveActive_ = true;
    dns_.start(53, "*", WiFi.softAPIP());

    for (const char* path : CAPTIVE_PATHS) {
        server_.on(path, [this]() { this->handleCaptiveRedirect(); });
    }
    server_.onNotFound([this]() { this->handleCaptiveRedirect(); });
}

void WebServer::stopCaptivePortal() {
    if (!captiveActive_) return;
    captiveActive_ = false;
    dns_.stop();
}
