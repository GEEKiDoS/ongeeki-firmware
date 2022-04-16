#pragma once

enum {
    DISK_BLOCK_NUM = 1024,
    DISK_BLOCK_SIZE = 512
};

extern const uint8_t disk_image[DISK_BLOCK_NUM * DISK_BLOCK_SIZE];
