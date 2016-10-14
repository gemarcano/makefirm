/*
 makefirm - Utility to make FIRM-like binaries out of ARM9/ARM11 payloads
 Wolfvak / fox8091
*/

#include "makefirm.h"
#include "sha256.h"


int sha_quick(u8 *dest, u8 *src, size_t src_len)
{
    SHA256_CTX *ctx = (SHA256_CTX*)malloc(sizeof(SHA256_CTX));
    if (ctx == NULL) return 1;
    sha256_init(ctx);
    sha256_update(ctx, src, src_len);
    sha256_final(ctx, dest);
    free(ctx);
    return 0;
}

static FirmHeader *firm_hdr = NULL;
static FILE *bin = NULL;

u8 *sect_buffer[4];
size_t sect_size[4];

int main(int argc, char *argv[])
{
    int ret = 0;
    if ((argc < 4) || (argc > 16) || ((argc - 4) % 3))
    {
        printf("Usage:  %s <output> <ARM11 entrypoint> <ARM9 entrypoint>\n\t[<section0.bin> <load_address> <0/1> (ARM9 = 0, ARM11 = 1)]\n\t...\n\t[<section4.bin> <load_address> <0/1>]\n", argv[0]);
        return 0;
    }

    char *firm_path = argv[1];
    u32 arm11_entrypoint = strtol(argv[2], NULL, 0), arm9_entrypoint = strtol(argv[3], NULL, 0);

    if (!arm11_entrypoint || !arm9_entrypoint)
    {
        printf("Invalid ARM11/ARM9 entrypoint!\n \
                ARM11 entrypoint: 0x%08X\n \
                ARM9 entrypoint: 0x%08X\n", arm11_entrypoint, arm9_entrypoint);
        ret = 1;
        goto end;
    }

    firm_hdr = (FirmHeader*)malloc(sizeof(FirmHeader));
    if (!firm_hdr)
    {
        printf("Couldn't allocate memory!\n");
        ret = 2;
        goto end;
    }

    firm_hdr->magic = FirmMagic;
    firm_hdr->arm11_entrypoint = arm11_entrypoint;
    firm_hdr->arm9_entrypoint = arm9_entrypoint;

    u32 payloads = (argc - 4) / 3;

    for (u32 i = 0; i < payloads; i++)
    {
        bin = fopen(argv[4 + (i * 3)], "rb");
        if (!bin)
        {
            printf("Couldn't open section %i file - %s!\n", i, argv[4 + (i * 3)]);
            ret = 3;
            goto end;
        }

        fseek(bin, 0L, SEEK_END);
        sect_size[i] = ftell(bin);
        rewind(bin);
        sect_buffer[i] = malloc(sect_size[i]);
        if (!sect_buffer[i])
        {
            fclose(bin);
            printf("Couldn't allocate %zu bytes of memory for section %d!\n", sect_size[i], i);
            ret = 4;
            goto end;
        }

        if (!fread(sect_buffer[i], sect_size[i], 1, bin))
        {
            fclose(bin);
            printf("Couldn't read section %i file - %s!\n", i, argv[4 + (i * 3)]);
            ret = 5;
            goto end;
        }

        fclose(bin);


        if (i == 0) firm_hdr->section[i].offset = sizeof(FirmHeader);
        else firm_hdr->section[i].offset = (firm_hdr->section[i-1].offset + sect_size[i-1]);


        firm_hdr->section[i].load_address = strtol(argv[4 + (i * 3) + 1], NULL, 0);
        if ((firm_hdr->section[i].load_address == 0) || (firm_hdr->section[i].load_address >= 0x30000000))
        {
            printf("Invalid section %i load address (0x%08X)!\n", i, firm_hdr->section[i].load_address);
            ret = 6;
            goto end;
        }


        firm_hdr->section[i].size = sect_size[i];
        u32 type = (u32)atoi(argv[4 + (i * 3) + 2]);
        if (type > 1)
        {
            printf("Invalid section %i type! Selected %i\n", i, type);
            ret = 7;
            goto end;
        }
        firm_hdr->section[i].type = type;
        sha_quick(firm_hdr->section[i].sha_hash, sect_buffer[i], sect_size[i]);
    }

    bin = fopen(firm_path, "wb");
    if (!bin)
    {
        fclose(bin);
        printf("Couldn't open %s for writing!\n", firm_path);
        ret = 7;
        goto end;
    }

    fwrite(firm_hdr, sizeof(FirmHeader), 1, bin);

    for (u32 i = 0; i < payloads; i++) fwrite(sect_buffer[i], sect_size[i], 1, bin);

    fclose(bin);

    printf("Created %s!\n", firm_path);

    end:

    free(firm_hdr);

    return ret;
}
