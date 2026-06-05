#ifdef _WIN32
#include <direct.h>
#include "dirent.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "zip.h"

int on_extract_entry(const char *filename, void *arg);
void remove_dir_recursive(const char *path);

int main(int argc, char* argv[])
{
	//extract section

	if (argc != 2) {
		printf("Command requires exactly one argument pointing to your 'The Dark Mod' install.\n");
		return 1;
	}

	char* output_dir;
	
	output_dir = malloc(strlen("tdm.pk3") +1);
	strcpy(output_dir, "tdm.pk3");

	#ifdef _WIN32
	_mkdir(output_dir);
	#else
	mkdir(output_dir, 0755);
	#endif

	printf("Output directory: %s\n", output_dir);

	struct dirent *entry;
	DIR *dir = opendir(argv[1]);

	if (dir == NULL) {
		perror("Failed to open TDM directory.");
		return 1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_REG) continue;

		char *ext = strrchr(entry->d_name, '.');
		if (ext && strcmp(ext, ".pk4") == 0) {
			char input_path[1024];
			snprintf(input_path, 
				sizeof(input_path), 
				"%s/%s", 
				argv[1], 
				entry->d_name);
			printf("\n\n\n\nProcessing: %s\n\n\n\n", input_path);

			int arg = 2;
			zip_extract(input_path, output_dir, on_extract_entry, &arg);
		}
	}	

	closedir(dir);

	// clean section
	
	int arr = 13;
	char* fn[13] = {
	"default.cfg",
//	TODO: delete all files here, "sound/*.sndshd"
	"af",
	"def",
	"fx",
	"guis",
	"prefabs",
	"script",
	"skins",
	"strings",
	"subtitles",
	"video",
	"xdata",
	};

	int i;
	for(i = 0; i < arr; i++) {
		char path[1024];
		snprintf(path,
			sizeof(path),
			"%s/%s",
			output_dir,
			fn[i]);

		int r = remove(path);
		if (r != 0) {
			remove_dir_recursive(path);
		}
		printf("\nRemoved: %s\n", path);
	}

	free(output_dir);

	printf("Extraction complete.\n");

	return 0;
}

void remove_dir_recursive(const char *path) {
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	struct dirent *p;

	if (!d) return;

	while ((p = readdir(d))) {
		char *buf;
		size_t len;

		if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
			continue;
		}

		len = path_len + strlen(p->d_name) + 2; 
		buf = malloc(len);
	
		if (buf) {
			struct stat statbuf;
			snprintf(buf, len, "%s/%s", path, p->d_name);
	
			if (!stat(buf, &statbuf)) {
				if (S_ISDIR(statbuf.st_mode)) {
					remove_dir_recursive(buf);
				} else {
					unlink(buf);
				}
			}
			free(buf);
		}
	}
	closedir(d);
    
	#ifdef _WIN32
	_rmdir(path);
	#else
	rmdir(path);
	#endif
}


int on_extract_entry(const char *filename, void *arg) {
	static int i = 0;
	int n = *(int *)arg;
	printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

	return 0;
}
