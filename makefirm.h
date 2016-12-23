#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FIRM_MAGIC (0x4D524946)

#define ALIGNMENT (16)

typedef struct {
    uint32_t offset;
    uint32_t ld_addr;
    uint32_t size;
    uint32_t type;
    uint8_t shahash[32];
} FirmSection;

typedef struct {
    uint32_t magic;
    uint8_t reserved1[4];
    uint32_t a11_entry;
    uint32_t a9_entry;
    uint8_t reserved2[48];
    FirmSection section[4];
    uint8_t rsa_sig[256];
} FirmHeader;

typedef enum
{
    ARM9 = 0,
    ARM11 = 1
} proc_type;
