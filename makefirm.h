#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define FirmMagic (0x4D524946)

typedef struct {
    u32 offset;
    u32 load_address;
    u32 size;
    u32 type;
    u8 sha_hash[0x20];
} FirmSection;

typedef struct {
    u32 magic;
    u8 reserved1[0x4];
    u32 arm11_entrypoint;
    u32 arm9_entrypoint;
    u8 reserved2[0x30];
    FirmSection section[0x4];
    u8 rsa_sig[0x100];
} FirmHeader;

typedef enum
{
    ARM9 = 0,
    ARM11 = 1
} proc_type;
