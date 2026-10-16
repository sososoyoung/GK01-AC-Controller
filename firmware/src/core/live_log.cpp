#include "core/live_log.h"
LiveLog appLog;
void LiveLog::tick() {
    if (!queue.active(millis())) { used_ = 0; truncated_ = false; memset(line_, 0, sizeof(line_)); }
}
uint32_t LiveLog::start(uint32_t session) { clear(); return queue.start(session, millis()); }
void LiveLog::clear() {
    queue.clear(); memset(line_, 0, sizeof(line_)); used_ = 0; truncated_ = false;
}
size_t LiveLog::write(uint8_t value) {
    Serial.write(value);
    tick();
    if (!queue.session()) return 1;
    if (value == '\n') {
        line_[used_] = 0;
        if (truncated_ && used_ >= 3) {
            size_t end = used_ - 3;
            while (end && (uint8_t(line_[end]) & 0xC0) == 0x80) --end;
            memcpy(line_ + end, "...", 4);
        }
        queue.append(line_, millis());
        memset(line_, 0, sizeof(line_)); used_ = 0; truncated_ = false;
    } else if (value != '\r') {
        if (used_ < sizeof(line_) - 1) line_[used_++] = char(value);
        else truncated_ = true;
    }
    return 1;
}
