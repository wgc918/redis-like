#pragma once
#include <vector>
#include <stdint.h>

class bufferPool
{
private:
    std::vector<uint8_t>& buf;

public:
    bufferPool(std::vector<uint8_t> &buf) : buf(buf) {}

    void buffer_append(const uint8_t *data, uint32_t len);
    void buffer_consume(uint32_t len);
};

