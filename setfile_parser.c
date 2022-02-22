#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define SET_FILE_MAGIC_NUMBER (0x12345679)

struct __setfile_header_ver_3 {
    uint32_t magic_number;
    uint32_t designed_bit;
    char     version_code[4];
    char     revision_code[4];
    uint32_t scenario_num;
    uint32_t subip_num;
    uint32_t setfile_offset;
}  __attribute__((__packed__));

union __setfile_header {
    uint32_t magic_number;
    struct __setfile_header_ver_3 ver_3;
};

static unsigned get_file_size (const char * fname) {
    struct stat sb;
    if (stat(fname, &sb) != 0) {
        fprintf(stderr, "'stat' failed for '%s': %s.\n", fname, strerror(errno));
        exit(1);
    }
    return sb.st_size;
}

static void* walk_file(const char* fname) {
    unsigned size;
    unsigned char* buff;
    size_t bytes_read;
    int status;
    FILE *f;

    size = get_file_size(fname);
    buff = malloc(size + 1);

    if (!buff) {
        fprintf(stderr, "Not enough memory.\n");
        goto failure;
    }

    f = fopen(fname, "r");
    if (!f) {
        fprintf(stderr, "Could not open '%s': %s.\n", fname, strerror(errno));
        goto failure;
    }

    bytes_read = fread(buff, sizeof(unsigned char), size, f);
    if (bytes_read != size) {
        fprintf (stderr, "Short read of '%s': expected %u bytes but got %zu: %s.\n", fname, size, bytes_read, strerror(errno));
        goto failure;
    }

    if (fclose(f) != 0) {
        fprintf (stderr, "Error closing '%s': %s.\n", fname, strerror(errno));
        goto failure;
    }

    return (void *) buff;

failure:
    exit(1);
}

/*
 * only one argument : setfile path
 */
int main(int argc, char const *argv[]) {
    void *addr;
    union __setfile_header *file_header;

    addr = walk_file(argv[1]);
    file_header = (union __setfile_header *) addr;

    if (file_header->magic_number == SET_FILE_MAGIC_NUMBER) {
        printf("version_code: %s\n", file_header->ver_3.version_code);
        printf("revision_code: %s\n", file_header->ver_3.revision_code);
    } else {
        printf("invalid magic number[0x%08x]\n", file_header->magic_number);
        goto failure;
    }

    return 0;

failure:
    exit(1);
}
