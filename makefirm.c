/*
 makefirm - Utility to make FIRM-like binaries out of ARM9/ARM11 payloads
 Wolfvak / fox8091
*/

#include "makefirm.h"
#include "sha256.h"

/* Quick wrapper around sha256.c */
int sha_quick(u8 *dest, u8 *src, size_t src_len)
{
    SHA256_CTX *ctx = (SHA256_CTX*)malloc(sizeof(SHA256_CTX));
    if (!ctx) return 1;
    sha256_init(ctx);
    sha256_update(ctx, src, src_len);
    sha256_final(ctx, dest);
    free(ctx);
    return 0;
}

/* Prevent leaks */
static void **buffers;
static size_t buf_ctr = 0;

void register_free(char *buf)
{
    buffers = realloc(buffers, sizeof(void *) * (buf_ctr + 1));
    buffers[buf_ctr++] = buf;
}

void atexit_free()
{
    for (int i = 0; i < buf_ctr; i++)
        free(buffers[i]);
    free(buffers);
}

static FirmHeader *firm_hdr = NULL;

u8 *sect_buffer[4];
size_t sect_size[4];

int main(int argc, char *argv[])
{
    if ((argc < 4) || (argc > 16) || ((argc - 4) % 3))
    {
        printf("Usage:  %s <output> <ARM11 entrypoint> <ARM9 entrypoint>\n\t[<section0.bin> <load_address> <0/1> (ARM9 = 0, ARM11 = 1)]\n\t...\n\t[<section4.bin> <load_address> <0/1>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *firm_path = argv[1];
    u32 arm11_entrypoint = strtol(argv[2], NULL, 0), arm9_entrypoint = strtol(argv[3], NULL, 0);

    if (!arm11_entrypoint || !arm9_entrypoint)
    { /* Check for invalid entrypoints */
        fprintf(stderr, "Invalid ARM11/ARM9 entrypoint!\nARM11 entrypoint: 0x%08X\nARM9 entrypoint: 0x%08X\n", arm11_entrypoint, arm9_entrypoint);
        return EXIT_FAILURE;
    }

    firm_hdr = (FirmHeader*)malloc(sizeof(FirmHeader));
    atexit(atexit_free);
    if (!firm_hdr)
    {
        fprintf(stderr, "Unable to allocate memory!\n");
        return EXIT_FAILURE;
    }
    register_free((char*)firm_hdr);

    firm_hdr->magic = FIRM_MAGIC;
    firm_hdr->arm11_entrypoint = arm11_entrypoint;
    firm_hdr->arm9_entrypoint = arm9_entrypoint;

    printf("FIRM information:\n\tARM11 entrypoint: 0x%08X\n\tARM9 entrypoint: 0x%08X\n\n", arm11_entrypoint, arm9_entrypoint);


    FILE *bin = NULL;
    u32 payloads = (argc - 4) / 3;
    for (u32 i = 0; i < payloads; i++)
    {
        char *payload_path = argv[4 + (i * 3)];
        u32 ld_addr = strtol(argv[4 + (i * 3) + 1], NULL, 0);

        bin = fopen(payload_path, "rb");
        if (!bin)
        {
            fprintf(stderr, "Unable to open file %s!\n", payload_path);
            return EXIT_FAILURE;
        }
        fseek(bin, 0, SEEK_END);
        sect_size[i] = ftell(bin);
        rewind(bin);

        sect_buffer[i] = malloc(sect_size[i]);
        if (!sect_buffer[i])
        {
            fclose(bin);
            fprintf(stderr, "Unable to allocate %zu bytes for section %d!\n", sect_size[i], i);
            return EXIT_FAILURE;
        }
        register_free((char*)sect_buffer[i]);

        if (!fread(sect_buffer[i], sect_size[i], 1, bin))
        {
            fclose(bin);
            fprintf(stderr, "Unable to read file %s!\n", payload_path);
            return EXIT_FAILURE;
        }

        fclose(bin);

        if (i == 0) firm_hdr->section[i].offset = sizeof(FirmHeader);
        else firm_hdr->section[i].offset = (firm_hdr->section[i-1].offset + sect_size[i-1]);


        firm_hdr->section[i].load_address = ld_addr;
        if (!ld_addr || (ld_addr >= 0x30000000))
        {
            fprintf(stderr, "Invalid load address at section %i (0x%08X)!\n", ld_addr, i);
            return EXIT_FAILURE;
        }

        firm_hdr->section[i].size = sect_size[i];

        u32 type = strtol(argv[4 + (i * 3) + 2], NULL, 0);
        if (type > 1)
        {
            fprintf(stderr, "Invalid section %i type! Selected %i\n", i, type);
            return EXIT_FAILURE;
        }

        firm_hdr->section[i].type = type;
        if (sha_quick(firm_hdr->section[i].sha_hash, sect_buffer[i], sect_size[i]))
        {
            fprintf(stderr, "Unable to calculate SHA256 hash of section %i! (out of memory?)\n", i);
            return EXIT_FAILURE;
        }

        printf("Section %i:\n\tOffset: 0x%08X\n\tLoad address: 0x%08X\n\tSize: 0x%08X\n\tType: %s\n\n", i, firm_hdr->section[i].offset, firm_hdr->section[i].load_address, firm_hdr->section[i].size, firm_hdr->section[i].type == 0 ? "ARM9" : "ARM11");
    }

    bin = fopen(firm_path, "wb");
    if (!bin)
    {
        fclose(bin);
        fprintf(stderr, "Unable to open %s for writing!\n", firm_path);
        return EXIT_FAILURE;
    }

    fwrite(firm_hdr, sizeof(FirmHeader), 1, bin);

    for (u32 i = 0; i < payloads; i++) fwrite(sect_buffer[i], sect_size[i], 1, bin);
    fclose(bin);

    printf("Created %s!\n", firm_path);
    return EXIT_SUCCESS;
}
