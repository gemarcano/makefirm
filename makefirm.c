/*
 makefirm - Utility to make FIRM-like binaries out of ARM9/ARM11 payloads
 Wolfvak / fox8091
*/

#include <stdarg.h>
#include <fcntl.h>

#include "makefirm.h"
#include "sha256.h"

void ABORT(const char *msg, ...)
{
    va_list vargs;
    va_start(vargs, msg);
    vfprintf(stderr, msg, vargs);
    va_end(vargs);
    exit(EXIT_FAILURE);
}

// Quick wrapper around sha256.c
int sha_quick(uint8_t *dest, uint8_t *src, size_t src_len)
{
    SHA256_CTX *ctx = (SHA256_CTX*)malloc(sizeof(SHA256_CTX));
    if (!ctx) ABORT("Couldn't calculate SHA256 hash!\n");
    sha256_init(ctx);
    sha256_update(ctx, src, src_len);
    sha256_final(ctx, dest);
    free(ctx);
    return 0;
}

int main(int argc, char *argv[])
{
    if ((argc < 4) || (argc > 16) || ((argc - 4) % 3))
        ABORT("Usage:  %s <out.bin> <ARM11 entry address> <ARM9 entry address>\n\t"
              "[<section0.bin> <ld_addr> <0/1> (ARM9 = 0, ARM11 = 1)]\n\t"
              "...\n\t"
              "[<section4.bin> <ld_addr> <0/1>]\n", argv[0]);

    char *firm_path = argv[1];
    uint32_t a11_entry = strtol(argv[2], NULL, 0), a9_entry = strtol(argv[3], NULL, 0);
    // strtol is used as it can parse most integer representations (decimal, hexadecimal, octal, etc)


    if (!a11_entry || !a9_entry) ABORT("Invalid entrypoints!\nARM11: %08X\nARM9: %08X\n", a11_entry, a9_entry);

    FirmHeader *firm_hdr = (FirmHeader*)malloc(sizeof(FirmHeader));
    if (!firm_hdr) ABORT("Unable to allocate FIRM header!\n");


    int firm_fd = open(firm_path, O_WRONLY | O_CREAT, 0644);
    if (firm_fd < 0) ABORT("Unable to open %s with write permissions\n", firm_path);
    lseek(firm_fd, 0x200, SEEK_SET);


    int n_payloads = ((argc - 4) / 3);
    for (int i = 0; i < n_payloads; i++)
    {
        FirmSection *section = &firm_hdr->section[i];

        section->offset  = lseek(firm_fd, 0, SEEK_CUR);

        char *payload_path = argv[(i * 3) + 4];
        section->ld_addr = strtol(argv[(i * 3) + 5], NULL, 0);
        section->type    = strtol(argv[(i * 3) + 6], NULL, 0) ? 1 : 0;

        int payload_fd = open(payload_path, O_RDONLY);
        if (payload_fd < 0) ABORT("Unable to open file %s!\n", payload_path);

        size_t payload_len = lseek(payload_fd, 0, SEEK_END);
        lseek(payload_fd, 0, SEEK_SET);
        payload_len = (payload_len % ALIGNMENT) ? ((payload_len - (payload_len & (ALIGNMENT-1))) + ALIGNMENT) : (payload_len);
        section->size = payload_len;

        uint8_t *payload_buf = (uint8_t*)malloc(payload_len);
        if (!payload_buf) ABORT("Couldn't allocate memory for payload %d!\n", i);
        memset(payload_buf, 0, payload_len);
        read(payload_fd, payload_buf, payload_len);
        sha_quick(section->shahash, payload_buf, payload_len);
        write(firm_fd, payload_buf, payload_len);
        close(payload_fd);

        free(payload_buf);

        printf("Section %i:\n\tOffset: 0x%08X\n\tLoad address: 0x%08X\n\tSize: 0x%08X\n\tType: %s\n\n",
            i, section->offset, section->ld_addr, section->size, section->type ? "ARM11" : "ARM9");
    }


    // Final FIRM setup
    firm_hdr->magic = FIRM_MAGIC;
    firm_hdr->a11_entry = a11_entry;
    firm_hdr->a9_entry = a9_entry;

    lseek(firm_fd, 0, SEEK_SET);
    write(firm_fd, firm_hdr, 0x200);
    close(firm_fd);

    free(firm_hdr);

    printf("FIRM information:\n\tARM11 entrypoint: 0x%08X\n\tARM9 entrypoint: 0x%08X\n\n", a11_entry, a9_entry);
    return EXIT_SUCCESS;
}


    
