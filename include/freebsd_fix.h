#pragma once

#include <time.h>

#define betoh16(x) bswap16((x))
#define betoh32(x) bswap32((x))
#define betoh64(x) bswap64((x))

#define letoh16(x) ((uint16_t)(x))
#define letoh32(x) ((uint32_t)(x))
#define letoh64(x) ((uint64_t)(x))

#define ENODATA 9919
