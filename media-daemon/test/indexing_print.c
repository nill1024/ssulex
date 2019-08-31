
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input>", argv[0]);
        exit(1);
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "fopen() failed -> %s.", argv[1]);
        exit(1);
    }

    if (fseek(input, 0, SEEK_END) != 0) {
        fprintf(stderr, "fseek() failed.");
        exit(1);
    }

    long long len = ftell(input);

    if (fseek(input, 0, SEEK_SET) != 0) {
        fprintf(stderr, "fseek() failed.");
        exit(1);
    }

    while (len) {
        int64_t timestamp;
        fread(&timestamp, sizeof(int64_t), 1, input);
        len -= sizeof(int64_t);
        fprintf(stdout, "%" PRId64 "\n", timestamp);
    }

    fclose(input);
    return 0;
}

