#include <aafc.h>
#include <tests.h>
#include <sstream>

int main(int argc, char* argv[]) {
    char** folders = NULL;
    unsigned long flen = 0;

    const char* outfilename = "fnoutp";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outfilename = argv[++i];
                printf("creating output at \"%s\"\n", outfilename);
            }
            else {
                if (folders != NULL)
                    free(folders);
                fprintf(stderr, "err: unspecified name\n");
                return 1;
            }
        }
        else {
            folders = (char**)realloc(folders, (flen + 1) * sizeof(char*));
            folders[flen] = argv[i];
            flen++;
        }
    }

    if (folders == nullptr) {
        fprintf(stderr, "No folders specified.\n");
        return 1;
    }

    unsigned short imported = 0;
    size_t size = 0;

    AFTInput* inp = (AFTInput*)calloc(flen, sizeof(AFTInput));
    for (unsigned char i = 0; i < flen; i++) {
        printf("processing group %d: \"%s\"\n", i+1, folders[i]);
        unsigned long len;
        char** files = list_files(folders[i], &len);
        if (files == NULL) {
            fprintf(stderr, "no files found\n");
            free(files);
            continue; // skip
        }

        inp[i].table = (AFTSubInput*)calloc(len, sizeof(AFTSubInput));
        inp[i].len = len;
        for (unsigned short j = 0; j < inp[i].len; j++) {
            AAFCOUTPUT dt = ReadFile(files[j]);
            if (!dt.data) continue; // skip
            inp[i].table[j].data = (unsigned char*)malloc(dt.size);
            inp[i].table[j].len = dt.size;
            char* dsc = filename_without_extension(files[j]);
            strncpy(inp[i].table[j].identifier, dsc, 255);
            free(dsc);
            memcpy(inp[i].table[j].data, dt.data, dt.size);
            imported++;
        }

        const char* groupn = strip_path_last(folders[i]);
        strncpy(inp[i].identifier, groupn, 63); // if i put it on top clang go angy

        free(files);
    }
    free(folders);

    if (imported == 0) {
        free(inp->table);
        free(inp);
        fprintf(stderr, "none inported\n");
        return 2;
    }



    AAFCTABLE ftable = CreateAFT(inp, flen);
    free(inp->table);
    free(inp);

    AAFCOUTPUT output = ExportAFT(&ftable);

    if (output.data == NULL) {
        fprintf(stderr, "failed\n");
        return 2;
    }

    mkdir("aft_packs/", 0755);

    char* c = concat_path_ext("aft_packs/", outfilename, "aft");
    FILE* ofile = fopen(c, "wb");
    if (ofile == NULL) {
        free(c);
        perror("aftcreator: failed to open/create output");
        return -1;
    }
    free(c);


    fwrite(output.data, 1, output.size, ofile);
    free(output.data);

    fclose(ofile);
    printf("Complete!\n");
    return 0;
}