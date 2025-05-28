#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void to_macro_name(const char* filename, char* buffer, size_t buflen) {
    size_t i = 0;
    for (; *filename && i < buflen - 1; filename++) {
        if ((*filename >= 'a' && *filename <= 'z') ||
            (*filename >= 'A' && *filename <= 'Z') ||
            (*filename >= '0' && *filename <= '9')) {
            buffer[i++] = (*filename >= 'a' && *filename <= 'z') ? *filename - 32 : *filename;
        } else {
            buffer[i++] = '_';
        }
    }
    buffer[i] = '\0';
}

int main(int argc, char** argv) {
    const char* in = NULL;
    const char* out = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f") && i + 1 < argc) {
            in = argv[++i];
        } else if (!strcmp(argv[i], "-o") && i + 1 < argc) {
            out = argv[++i];
        }
    }

    if (!in || !out) {
        fprintf(stderr, "Usage: %s -f inputfile -o outputfile.h\n", argv[0]);
        return 1;
    }

    FILE* fin = fopen(in, "rb");
    if (!fin) {
        perror("fopen input");
        return 1;
    }

    fseek(fin, 0, SEEK_END);
    long size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    uint8_t* data = malloc(size);
    if (!data) {
        fclose(fin);
        perror("malloc");
        return 1;
    }

    if (fread(data, 1, size, fin) != (size_t)size) {
        fclose(fin);
        free(data);
        perror("fread");
        return 1;
    }

    fclose(fin);

    FILE* fout = fopen(out, "w");
    if (!fout) {
        perror("fopen output");
        free(data);
        return 1;
    }

    char macro[256];
    to_macro_name(out, macro, sizeof(macro));

    fprintf(fout, "#ifndef %s\n#define %s 1\n\n", macro, macro);
    fprintf(fout, "const uint8_t %s[] = {\n", in);

    for (long i = 0; i < size; i++) {
        fprintf(fout, "0x%02X%s", data[i], (i + 1 < size) ? ", " : "");
        if ((i + 1) % 12 == 0) fprintf(fout, "\n");
    }

    fprintf(fout, "\n};\n\n#endif /* %s */\n", macro);
    fclose(fout);
    free(data);

    return 0;
}
