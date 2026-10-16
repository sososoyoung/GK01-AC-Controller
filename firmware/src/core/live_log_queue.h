#pragma once
#include <stdint.h>
#include <stddef.h>

// Transport-only RAM, never a history store. No Arduino, network or FS dependency.
class LiveLogQueue {
public:
    static constexpr size_t CAPACITY = 20;
    static constexpr size_t TEXT_SIZE = 176;
    static constexpr uint32_t LEASE_MS = 7000;
    struct Entry { uint32_t seq, uptime; char text[TEXT_SIZE]; };
    uint32_t start(uint32_t session, uint32_t now);
    bool renew(uint32_t session, uint32_t now);
    bool stop(uint32_t session);
    void clear();
    void tick(uint32_t now);
    bool active(uint32_t now);
    void append(const char* text, uint32_t now);
    uint32_t session() const { return session_; }
    uint32_t cursor() const { return sequence_; }
    size_t size() const { return count_; }
    const Entry& at(size_t index) const { return entries_[(head_ + index) % CAPACITY]; }
    // Discard acknowledged entries; report only entries lost since this cursor.
    uint32_t acknowledge(uint32_t after);
private:
    Entry entries_[CAPACITY]{};
    uint32_t session_ = 0, sequence_ = 0, touched_ = 0;
    size_t head_ = 0, count_ = 0;
};
