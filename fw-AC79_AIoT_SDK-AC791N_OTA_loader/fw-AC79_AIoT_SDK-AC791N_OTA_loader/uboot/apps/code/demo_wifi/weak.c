#include "utils/endian.h"

__attribute__((weak)) u16 _swap16(u16 value)
{
    return _SWAP16(value);
}

__attribute__((weak)) u32 _swap32(u32 value)
{
    return _SWAP32(value);
}

__attribute__((weak)) u64 _swap64(u64 value)
{
    return _SWAP64(value);
}

