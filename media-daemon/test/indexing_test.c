
#include <stdio.h>
#include <stdlib.h>
#include <indexing.h>
#include <libavformat/avformat.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        exit(1);
    }

    FILE *output = fopen("output", "wb");
    if (output == NULL) {
        fprintf(stderr, "fopen() failed.\n");
        exit(1);
    }

    av_register_all();

    int ret;
    if ((ret = index_key_frames(output, argv[1])) < 0) {
        fprintf(stderr, "index_key_frames() failed -> %s.", argv[1]);
        fclose(output);
        exit(1);
    }

    fclose(output);
    return 0;
}

