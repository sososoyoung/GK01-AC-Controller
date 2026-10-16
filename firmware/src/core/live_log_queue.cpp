#include "core/live_log_queue.h"
#include <string.h>
uint32_t LiveLogQueue::start(uint32_t session, uint32_t now) {
    clear(); session_ = session ? session : 1; touched_ = now; return sequence_;
}
bool LiveLogQueue::renew(uint32_t session, uint32_t now) {
    tick(now);
    if (!session || session != session_) return false;
    touched_ = now; return true;
}
bool LiveLogQueue::stop(uint32_t session) {
    if (!session || session != session_) return false;
    clear(); return true;
}
void LiveLogQueue::clear() {
    session_ = 0; head_ = count_ = 0;
    memset(entries_, 0, sizeof(entries_));
}
void LiveLogQueue::tick(uint32_t now) {
    if (session_ && uint32_t(now - touched_) >= LEASE_MS) clear();
}
bool LiveLogQueue::active(uint32_t now) { tick(now); return session_ != 0; }
void LiveLogQueue::append(const char* text, uint32_t now) {
    if (!active(now)) return;
    if (count_ == CAPACITY) { head_ = (head_ + 1) % CAPACITY; --count_; }
    Entry& entry = entries_[(head_ + count_++) % CAPACITY];
    entry.seq = ++sequence_; entry.uptime = now;
    size_t length = strlen(text), out = 0;
    for (size_t i = 0; i < length && out < TEXT_SIZE - 1;) {
        uint8_t c = uint8_t(text[i]);
        size_t bytes = c < 0x80 ? 1 : c >= 0xC2 && c <= 0xDF ? 2 :
                       c >= 0xE0 && c <= 0xEF ? 3 : c >= 0xF0 && c <= 0xF4 ? 4 : 0;
        bool valid = bytes && i + bytes <= length;
        for (size_t j = 1; valid && j < bytes; ++j) valid = (uint8_t(text[i + j]) & 0xC0) == 0x80;
        if (valid && bytes >= 3) {
            uint8_t second = uint8_t(text[i + 1]);
            valid = !(c == 0xE0 && second < 0xA0) && !(c == 0xED && second >= 0xA0) &&
                    !(c == 0xF0 && second < 0x90) && !(c == 0xF4 && second >= 0x90);
        }
        if (!valid) { entry.text[out++] = '?'; ++i; continue; }
        if (out + bytes >= TEXT_SIZE) break;
        memcpy(entry.text + out, text + i, bytes); i += bytes; out += bytes;
    }
    entry.text[out] = '\0';
}
uint32_t LiveLogQueue::acknowledge(uint32_t after) {
    while (count_ && int32_t(at(0).seq - after) <= 0) {
        memset(&entries_[head_], 0, sizeof(Entry));
        head_ = (head_ + 1) % CAPACITY; --count_;
    }
    return count_ && int32_t(at(0).seq - after) > 1 ? at(0).seq - after - 1 : 0;
}
