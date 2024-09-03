#include <aafc.h>
#include <tests.h>
#include <sstream>

int main(int argc, char* argv[]) 
{

#if 0
    char** folders = nullptr;
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

    // TODO: REWORK FILE TABLES
    AAFCFILETABLE ftable{};

    // CREATE FILETABLE HERE

    AAFCOUTPUT output = ExportAFT(&ftable);

    // FREE DATA HERE

    if (output.data == NULL) {
        printf("Failed.\n");
        return 2;
    }

    mkdir("aft_packs/", 0755);

    char* c = concat_path_aft("aft_packs/", outfilename);
    FILE* ofile = fopen(c, "wb");
    if (ofile == NULL) {
        free(c);
        perror("aftcreator: failed to open/creat output");
        return -1;
    }
    free(c);


    fwrite(output.data, sizeof(unsigned char), output.size, ofile);
    free((void*)output.data);

    printf("Complete!\n");
#endif
    return 0;
}