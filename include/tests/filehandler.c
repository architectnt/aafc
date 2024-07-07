#include <fstream>
#include <aafc.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>

// cross-platform defines
#define mkdir(dir, mode) _mkdir(dir)
#define strdup _strdup

#else
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <dirent.h>
#endif

char** list_files(const char* dir, int* len) {
	char** files = NULL;
	*len = 0;
	char abs_file_path[512];

#ifdef _WIN32
	char fullpath[512];

	snprintf(fullpath, sizeof(fullpath), "%s/*", dir);
	WIN32_FIND_DATA fdt;
	HANDLE fhndl = FindFirstFile(fullpath, &fdt);

	if (fhndl == INVALID_HANDLE_VALUE) {
		printf("cant get the first file >:((((((((((((((((: %d\n", GetLastError());
		return NULL;
	}
	do {
		if (!(fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			snprintf(abs_file_path, sizeof(abs_file_path), "%s\\%s", dir, fdt.cFileName);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(abs_file_path);
			(*len)++;
		}
	} while (FindNextFile(fhndl, &fdt) != 0);
	FindClose(fhndl);

#else
	DIR* dirc;
	struct dirent* entry;

	if (!(dirc = opendir(dir)))
		return NULL;

	while ((entry = readdir(dirc)) != NULL) {
		if (entry->d_type == DT_REG) {
			snprintf(abs_file_path, sizeof(abs_file_path), "%s/%s", dir, entry->d_name);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(abs_file_path);
			(*len)++;
		}
	}
	closedir(dirc);
#endif

	return files;
}

char* filename_without_extension(const char* path) {
	const char* base = strrchr(path, '/');
	if (!base) {
		base = strrchr(path, '\\');
	}
	if (!base) {
		base = path;
	}
	else {
		base++;
	}

	const char* ext = strrchr(base, '.');
	size_t len;
	if (!ext) {
		len = strlen(base);
	}
	else {
		len = ext - base;
	}

	char* filename = (char*)malloc(len + 1);
	if (!filename) {
		perror("Failed to allocate memory");
		return NULL;
	}

	strncpy(filename, base, len);
	filename[len] = '\0';

	return filename;
}

char* get_dir_name(const char* path) {
	if (path == NULL || *path == '\0') {
		return NULL;
	}

	const char* last_slash = strrchr(path, '/');
	if (!last_slash) {
		last_slash = strrchr(path, '\\');
	}

	if (!last_slash) {
		return strdup(path);
	}

	if (*(last_slash + 1) == '\0') {
		while (last_slash > path) {
			last_slash--;
			if (*last_slash == '/' || *last_slash == '\\') {
				last_slash++;
				break;
			}
		}

		if (last_slash == path && (*last_slash != '/' && *last_slash != '\\')) {
			return strdup(path);
		}
	}
	else {
		last_slash++;
	}

	size_t len = strlen(last_slash);
	char* dirname = (char*)malloc(len + 1);
	if (!dirname) {
		perror("Failed to allocate memory");
		return NULL;
	}

	strncpy(dirname, last_slash, len);
	dirname[len] = '\0';

	return dirname;
}


char* add_fext(const char* filename, const char* extension) {
	size_t filename_len = strlen(filename);
	size_t extension_len = strlen(extension);

	char* new_filename = (char*)malloc(filename_len + extension_len + 1);
	if (new_filename == NULL) {
		perror("Failed to allocate memory");
		return NULL;
	}
	strcpy(new_filename, filename);
	strcat(new_filename, extension);

	return new_filename;
}


AAFCOUTPUT ReadFile(const char* path) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		perror("cant open aafc file :(");
		return {};
	}

	fseek(file, 0, SEEK_END);
	size_t fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	unsigned char* data = (unsigned char*)malloc(fsize);
	if (data == NULL) {
		perror("MEMORY ALLOC INTERNAL ERROR");
		fclose(file);
		return {};
	}

	size_t byrd = fread(data, 1, fsize, file);
	if (byrd != fsize) {
		perror("error trying to load the file");
		fclose(file);
		return {};
	}

	fclose(file);
	return {data, fsize};
}