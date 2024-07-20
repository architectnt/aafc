#include <aafc.h>
#include <tests.h>
#include <sstream>

int main(int argc, char* argv[]) 
{
    // TODO: REWORK FILE TABLES

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

    printf("Packing sounds.. ");

    AAFCFILETABLE ftable {};
	ftable.size = argc - 1;
	ftable.filetables = (FILETABLE*)malloc(ftable.size * sizeof(FILETABLE));
    size_t current_offset = 0;

    for (unsigned char i = 0; i < ftable.size; i++) {
        int file_count;
        char** files = list_files(folders[i], &file_count);

        ftable.filetables[i].size = file_count;
        ftable.filetables[i].table = (size_t*)malloc(file_count * sizeof(size_t));
        ftable.filetables[i].data = (DATATABLE*)malloc(file_count * sizeof(DATATABLE));

        for (int j = 0; j < file_count; j++) {
            AAFCOUTPUT file = ReadFile(files[j]);
            ftable.filetables[i].table[j] = current_offset;
            ftable.filetables[i].data[j].len = file.size;
            ftable.filetables[i].data[j].data = file.data;

            current_offset += file.size;

            free(files[j]);
        }
        free(files);
    }

    printf("Finalizing.. \n");

    AAFCOUTPUT output = ExportAFT(&ftable);

    for (unsigned char i = 0; i < ftable.size; i++) {
        for (unsigned short j = 0; j < ftable.filetables[i].size; j++) {
            free(ftable.filetables[i].data[j].data);
        }
        free(ftable.filetables[i].table);
        free(ftable.filetables[i].data);
    }
    free(ftable.filetables);

    if (output.data == NULL) {
        printf("Failed.\n");
        return 2;
    }

    mkdir("aft_packs/", 0755);
    std::stringstream fbp; fbp << "aft_packs/" << outfilename << ".aft";

    FILE* ofile = fopen(fbp.str().c_str(), "wb");
    if (ofile == NULL) {
        perror("aud2aafc: failed to open output file >:((((");
        return -1;
    }

    fwrite(output.data, sizeof(unsigned char), output.size, ofile);
    free((void*)output.data);

    printf("Complete!\n");

#endif
    return 0;
}