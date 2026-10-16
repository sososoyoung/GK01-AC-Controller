#pragma once
#include <Arduino.h>
#include "core/live_log_queue.h"

// Application code uses the standard Print interface. Serial keeps full output;
// only complete bounded lines enter the on-demand web transport queue.
class LiveLog : public Print {
public:
    using Print::write;
    size_t write(uint8_t value) override;
    void tick();
    uint32_t start(uint32_t session);
    void clear();
    LiveLogQueue queue;
private:
    char line_[LiveLogQueue::TEXT_SIZE]{};
    size_t used_ = 0;
    bool truncated_ = false;
};
static_assert(sizeof(LiveLog) <= 4096, "Live log RAM must stay within 4 KiB");
extern LiveLog appLog;
