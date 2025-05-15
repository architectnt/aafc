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
	char afp[512];

#ifdef _WIN32
	char fullpath[512];

	snprintf(fullpath, sizeof(fullpath), "%s/*", dir);
	WIN32_FIND_DATA fdt;
	HANDLE fhndl = FindFirstFile(fullpath, &fdt);

	if (fhndl == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	while (FindNextFile(fhndl, &fdt) != 0){
		if (!(fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			snprintf(afp, sizeof(afp), "%s\\%s", dir, fdt.cFileName);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(afp);
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
			snprintf(afp, sizeof(afp), "%s/%s", dir, entry->d_name);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(afp);
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

static char* concat_path_ext(const char* dir, const char* filename, const char* ext) {
	if (!dir || !filename || !ext || *dir == '\0' || *filename == '\0' || *ext == '\0')
		return NULL;

	size_t len = strlen(dir) + 1 + strlen(filename) + 1 + strlen(ext) + 1;
	char* result = (char*)malloc(len);
	if (result)
		snprintf(result, len, "%s/%s.%s", dir, filename, ext);
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

static AAFCOUTPUT ReadAAFCFile(const char* path) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		perror("cant open aafc file :(");
		return (AAFCOUTPUT) { 0, NULL };
	}

	fseek(file, 0, SEEK_END);
	size_t fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	unsigned char* data = (unsigned char*)malloc(fsize);
	if (data == NULL) {
		perror("MEMORY ALLOC INTERNAL ERROR");
		fclose(file);
		return (AAFCOUTPUT) { 0, NULL };
	}

	if (fread(data, 1, fsize, file) != fsize) {
		perror("error trying to load the file");
		fclose(file);
		return (AAFCOUTPUT) { 0, NULL };
	}

	fclose(file);
	return (AAFCOUTPUT) {fsize, data};
}