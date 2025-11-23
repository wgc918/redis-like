#include "bufferPool.h"

void bufferPool::buffer_append(const uint8_t *data, uint32_t len)
{
    buf.insert(buf.end(), data, data + len);
}

void bufferPool::buffer_consume(uint32_t len)
{
    buf.erase(buf.begin(), buf.begin() + len);
}
