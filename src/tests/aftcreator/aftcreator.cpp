#include <aafc.h>
#include <tests.h>
#include <sstream>

int main(int argc, char* argv[]) {
    char** folders = NULL;
    const char* outfilename = "fnoutp";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            outfilename = argv[i++];
        }
        else {
            folders = argv + i;
            break;
        }
    }

    if (folders == nullptr) {
        std::cerr << "No folders specified." << std::endl;
        return 1;
    }


    unsigned long len;
    char** files = list_files(folders[0], &len);
    if (!files) {
        std::cerr << "no files found" << std::endl;
        return 2;
    }

    AFTInput* inp = (AFTInput*)calloc(1, sizeof(AFTInput)); // for now
    inp->len = len;
    inp->table = (AFTSubInput*)calloc(len, sizeof(AFTSubInput));

    unsigned short imported = 0;
    for (unsigned short i = 0; i < inp->len; i++) {
        AAFCOUTPUT dt = ReadFile(files[i]);
        if (!dt.data) continue; // skip
        inp->table[i].data = (unsigned char*)malloc(dt.size);
        inp->table[i].len = dt.size;
        char* dsc = filename_without_extension(files[i]);
        strncpy(inp->table[i].identifier, dsc, 255);
        free(dsc);
        memcpy(inp->table[i].data, dt.data, dt.size);
        imported++;
    }
    free(files);

    AAFCTABLE ftable = CreateAFT(inp, 1);
    free(inp->table);
    free(inp);

    AAFCOUTPUT output = ExportAFT(&ftable);

    if (output.data == NULL) {
        printf("Failed.\n");
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

    printf("Complete!\n");
    return 0;
}