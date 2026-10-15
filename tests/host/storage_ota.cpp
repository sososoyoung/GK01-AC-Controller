#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <utility>
#include <vector>

using std::min;
using String = std::string;
#define F(x) x
struct Logger {
    void println(const char*) {}
    template<class... T> void printf(const char*, T...) {}
} Serial;
uint32_t millis() { return 100; }

struct FakeFS;
struct File {
    FakeFS* fs = nullptr;
    std::string path;
    explicit operator bool() const { return fs != nullptr; }
    size_t printf(const char* format, ...);
    void close() {}
};
struct FakeFS {
    std::map<std::string, std::string> files;
    std::string failRenameFrom;
    bool failOpen = false;
    bool exists(const char* path) { return files.count(path) != 0; }
    bool remove(const char* path) { return files.erase(path) != 0; }
    bool rename(const char* from, const char* to) {
        if (!exists(from) || failRenameFrom == from) return false;
        files[to] = files[from];
        files.erase(from);
        return true;
    }
    File open(const char* path, const char*) {
        if (failOpen) return {};
        files[path].clear();
        return {this, path};
    }
} LittleFS;
size_t File::printf(const char* format, ...) {
    char text[256];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    assert(size >= 0 && size < int(sizeof(text)));
    fs->files[path] += text;
    return size;
}
bool mountOk = true;
bool ensureFSMounted(bool) { return mountOk; }

#include "config/config_store.h"
#include "ota/ota_manager.h"

constexpr uint32_t ROM0_ADDRESS = 0x2000;
constexpr uint32_t ROM1_ADDRESS = 0x102000;
uint8_t currentRom = 0;
uint32_t addressOverride = 0;
uint8_t rboot_get_current_rom() { return currentRom; }
uint8_t rboot_get_target_rom() { return currentRom ^ 1; }
uint32_t rboot_get_rom_address(uint8_t rom) {
    return addressOverride ? addressOverride : (rom ? ROM1_ADDRESS : ROM0_ADDRESS);
}
std::vector<uint8_t> flash(0x400000, 0xFF);
std::vector<std::pair<uint32_t, size_t>> writes;
bool failWrite = false;
bool rboot_write_flash(uint32_t addr, const uint8_t* data, size_t len) {
    const uint32_t target = currentRom ? ROM0_ADDRESS : ROM1_ADDRESS;
    assert(addr >= target && addr + len <= target + 0xFE000);
    assert(addr % 4096 == 0);
    if (failWrite) return false;
    writes.push_back({addr, len});
    // Erase whole sectors, as the production driver does. An overlapping
    // partial write would destroy preceding bytes and fail the final compare.
    size_t end = (addr + len + 4095) / 4096 * 4096;
    std::fill(flash.begin() + addr, flash.begin() + end, 0xFF);
    std::copy(data, data + len, flash.begin() + addr);
    return true;
}
enum { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };
struct HTTPUpload {
    int status = UPLOAD_FILE_START;
    uint8_t buf[8192];
    size_t currentSize = 0;
};
struct FakeServer {
    HTTPUpload request;
    bool authenticated = false;
    HTTPUpload& upload() { return request; }
    bool authenticate(const char*, const char*) { return authenticated; }
};
struct FakeWeb {
    FakeServer server;
    FakeServer& raw() { return server; }
} web;
const char* getApPass() { return "test-password"; }
OtaManager ota;
constexpr uint32_t SPI_FLASH_SEC_SIZE = 4096;
constexpr int SPI_FLASH_RESULT_OK = 0;
void yield() {}
void noInterrupts() {}
void interrupts() {}
int spi_flash_erase_sector(uint32_t sector) {
    auto start = flash.begin() + sector * SPI_FLASH_SEC_SIZE;
    std::fill(start, start + SPI_FLASH_SEC_SIZE, 0xFF);
    return SPI_FLASH_RESULT_OK;
}
int spi_flash_write(uint32_t addr, uint32_t* data, size_t len) {
    assert((addr % 4) == 0 && (len % 4) == 0);
    memcpy(flash.data() + addr, data, len);
    return SPI_FLASH_RESULT_OK;
}
#include "methods.inc"

std::vector<uint8_t> image(size_t size = 130123) {
    std::vector<uint8_t> result(size);
    for (size_t i = 0; i < size; ++i) result[i] = uint8_t(i * 17);
    result[0] = 0xEA; result[1] = 4; result[2] = 3;
    uint32_t zero = 0, iromSize = 100000;
    memcpy(result.data() + 8, &zero, 4);
    memcpy(result.data() + 12, &iromSize, 4);
    return result;
}
void resetUpload() {
    ota = OtaManager{};
    writes.clear();
    failWrite = false;
    addressOverride = 0;
    std::fill(flash.begin(), flash.end(), 0xFF);
}
void send(int status, const uint8_t* data = nullptr, size_t len = 0) {
    auto& req = web.server.request;
    req.status = status;
    req.currentSize = len;
    assert(len <= sizeof(req.buf));
    if (len) memcpy(req.buf, data, len);
    handleUpload();
}

int main() {
    ConfigStore config;
    strcpy(config.cfg.sta_ssid, "test-network");
    assert(config.save());
    assert(LittleFS.files.at(FS_CONFIG_TXT).find("ssid=test-network\n") != String::npos);
    assert(LittleFS.files.size() == 1);
    strcpy(config.cfg.sta_ssid, "updated-network");
    assert(config.save());
    const auto saved = LittleFS.files.at(FS_CONFIG_TXT);
    assert(saved.find("ssid=updated-network\n") != String::npos);
    LittleFS.failRenameFrom = "/config.txt.tmp";
    strcpy(config.cfg.sta_ssid, "must-not-commit");
    assert(!config.save());
    assert(LittleFS.files.at(FS_CONFIG_TXT) == saved);
    LittleFS.failRenameFrom = FS_CONFIG_TXT;
    assert(!config.save());
    assert(LittleFS.files.at(FS_CONFIG_TXT) == saved);
    LittleFS.failRenameFrom.clear();
    LittleFS.failOpen = true;
    assert(!config.save());
    LittleFS.failOpen = false;
    mountOk = false;
    assert(!config.save());
    assert(!config.isMounted());
    mountOk = true;
    assert(config.save());
    assert(config.isMounted());
    puts("PASS config: first save, overwrite, commit/backup/open/mount failures, recovery");

    auto data = image();
    resetUpload();
    web.server.authenticated = false;
    send(UPLOAD_FILE_START);
    for (int i = 0; i < 4; ++i) send(UPLOAD_FILE_WRITE, data.data() + i * 2048, 2048);
    send(UPLOAD_FILE_END);
    assert(writes.empty() && !ota.lastUploadOk());
    assert(!ota.onUploadChunk(data.data(), 8192));
    assert(writes.empty());
    puts("PASS OTA: unauthenticated request and write without START cannot touch flash");

    for (currentRom = 0; currentRom < 2; ++currentRom) {
        for (size_t chunkSize : {size_t(1), size_t(1460), size_t(2048), size_t(4096), size_t(7000)}) {
            resetUpload();
            web.server.authenticated = true;
            send(UPLOAD_FILE_START);
            for (size_t pos = 0; pos < data.size(); pos += chunkSize) {
                send(UPLOAD_FILE_WRITE, data.data() + pos, min(chunkSize, data.size() - pos));
            }
            send(UPLOAD_FILE_END);
            assert(ota.lastUploadOk() && ota.lastUploadSize() == data.size());
            const uint32_t target = currentRom ? ROM0_ADDRESS : ROM1_ADDRESS;
            assert(std::equal(data.begin(), data.end(), flash.begin() + target));
            for (size_t i = 0; i + 1 < writes.size(); ++i) assert(writes[i].second == 4096);
            auto count = writes.size();
            assert(!ota.onUploadChunk(data.data(), 2048));
            assert(writes.size() == count);
        }
    }
    currentRom = 0;
    puts("PASS OTA: both slots, five chunk sizes, byte-for-byte image, no writes after END");

    resetUpload();
    send(UPLOAD_FILE_START);
    send(UPLOAD_FILE_WRITE, data.data(), 2048);
    send(UPLOAD_FILE_ABORTED);
    send(UPLOAD_FILE_WRITE, data.data(), 8192);
    assert(writes.empty() && !ota.lastUploadOk());
    send(UPLOAD_FILE_START);
    send(UPLOAD_FILE_WRITE, data.data(), 8192);
    assert(writes.size() == 2);
    const auto count = writes.size();
    web.server.authenticated = false;
    send(UPLOAD_FILE_START);
    send(UPLOAD_FILE_WRITE, data.data(), 8192);
    send(UPLOAD_FILE_END);
    assert(writes.size() == count && !ota.lastUploadOk());
    puts("PASS OTA: abort, retry, unauthenticated request after an authenticated upload");

    resetUpload();
    web.server.authenticated = true;
    addressOverride = ROM0_ADDRESS;
    send(UPLOAD_FILE_START);
    send(UPLOAD_FILE_WRITE, data.data(), 8192);
    assert(writes.empty() && !ota.lastUploadOk());
    resetUpload();
    send(UPLOAD_FILE_START);
    auto bad = data;
    bad[0] = 0xE9;
    send(UPLOAD_FILE_WRITE, bad.data(), 8192);
    assert(writes.empty() && !ota.lastUploadOk());
    resetUpload();
    send(UPLOAD_FILE_START);
    assert(!ota.onUploadChunk(data.data(), 0xFE001));
    assert(writes.empty());
    resetUpload();
    send(UPLOAD_FILE_START);
    failWrite = true;
    send(UPLOAD_FILE_WRITE, data.data(), 8192);
    assert(!ota.lastUploadOk());
    failWrite = false;
    send(UPLOAD_FILE_WRITE, data.data(), 8192);
    send(UPLOAD_FILE_END);
    assert(writes.empty());
    puts("PASS OTA: invalid target/header/size and failed flash write block further writes");
    for (currentRom = 0; currentRom < 2; ++currentRom) {
        resetUpload();
        uint32_t target = currentRom ? ROM0_ADDRESS : ROM1_ADDRESS;
        assert(!driver_write_flash(0, data.data(), 4096));
        assert(!driver_write_flash(currentRom ? ROM1_ADDRESS : ROM0_ADDRESS, data.data(), 4096));
        assert(!driver_write_flash(target + 1, data.data(), 4096));
        assert(!driver_write_flash(target + 0xFE000, data.data(), 4096));
        assert(!driver_write_flash(target, data.data(), 0xFE001));
        assert(std::all_of(flash.begin(), flash.end(), [](uint8_t b) { return b == 0xFF; }));
        assert(driver_write_flash(target, data.data(), 4096));
        assert(driver_write_flash(target + 4096, data.data() + 4096, 11));
        assert(std::equal(data.begin(), data.begin() + 4107, flash.begin() + target));
    }
    puts("PASS flash driver: boot/active/FS/unaligned/oversize writes refused, inactive slot writable");
}
