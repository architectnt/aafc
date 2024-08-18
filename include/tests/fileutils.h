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
#include <dirent.h>
#endif

static char** list_files(const char* dir, unsigned int* len) {
	char** files = NULL;
	*len = 0;
	char abs_file_path[512];

#ifdef _WIN32
	char fullpath[512];

	snprintf(fullpath, sizeof(fullpath), "%s/*", dir);
	WIN32_FIND_DATA fdt;
	HANDLE fhndl = FindFirstFile(fullpath, &fdt);

	if (fhndl == INVALID_HANDLE_VALUE) {
		printf("cant get the first file >:((((((((((((((((: %lu\n", GetLastError());
		return NULL;
	}
	while (FindNextFile(fhndl, &fdt) != 0){
		if (!(fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			snprintf(abs_file_path, sizeof(abs_file_path), "%s\\%s", dir, fdt.cFileName);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(abs_file_path);
			(*len)++;
		}
	}
	FindClose(fhndl);

#else
	DIR* dirc;
	dirent* entry;

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

static const char* strip_path_last(const char* path) {
	if (path == NULL || *path == '\0')
		return NULL;

	const char* lsep = NULL;
	const char* p = path;

	while (*p) {
		if (*p == '/' || *p == '\\')
			lsep = p;
		p++;
	}

	if (lsep == NULL)
		return path;

	return lsep + 1;
}

static char* concat_path(const char* dir, const char* filename) {
	if (dir == NULL || filename == NULL || *dir == '\0' || *filename == '\0')
		return NULL;

	size_t len = (strlen(dir) + strlen(filename) + strlen(".aafc/")) + 1;
	char* result = (char*)malloc(len);
	if (result)
		snprintf(result, len, "%s/%s.aafc", dir, filename);
	return result;
}

static char* filename_without_extension(const char* path) {
	if (path == NULL || *path == '\0')
		return NULL;

	const char* lsep = NULL;
	const char* p = path;

	while (*p) {
		if (*p == '/' || *p == '\\')
			lsep = p;
		p++;
	}

	const char* fstr = (lsep != NULL) ? lsep + 1 : path;
	const char* lped = strrchr(fstr, '.');
	size_t flen = (lped != NULL) ? (size_t)(lped - fstr) : strlen(fstr);

	char* result = (char*)malloc(flen + 1);
	if (result == NULL)
		return NULL;

	strncpy(result, fstr, flen);
	result[flen] = '\0';

	return result;
}

static AAFCOUTPUT ReadFile(const char* path) {
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

	if (fread(data, 1, fsize, file) != fsize) {
		perror("error trying to load the file");
		fclose(file);
		return {};
	}

	fclose(file);
	return {fsize, data};
}