#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include "dirent.h"
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "zip.h"

int on_extract_entry(const char *filename, void *arg);
int is_directory(const char *path);
int is_file(const char *path);

void rename_non_d_folders(const char *textures_path);
void remove_dir_recursive(const char *path);
void flatten_recursive	(const char *dest_path, const char *src_path, const char *prefix);
void remove_empty_dirs	(const char *path);
void ensure_directory	(const char *path);
void scan_and_move	(const char *textures_path, const char *current_path,
			 const char *folder_name, const char *dest_dir_name);

int path_max = 4096;


int main(int argc, char* argv[])
{
	//extract section

	if (argc != 2) {
		printf("Command requires exactly one argument pointing to your 'The Dark Mod' install.\n");
		return 1;
	}

	char* output_dir;
	// placeholder until its needed to specify the output dir
	const char* tmpstr = "tdm.pk3dir";
	
	output_dir = malloc(strlen(tmpstr) +1);
	strcpy(output_dir, tmpstr);

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
	
	int arr = 12;
	char* fn[12] = {
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

	// handling of folder.pk3dir/textures/darkmod/ to folder.pk3dir/textures/d_foldername(s)
	
	char darkmod_src [1024];
	char darkmod_dest[1024];

	snprintf(darkmod_src , sizeof(darkmod_src) , "%s/textures/darkmod/", output_dir);
	snprintf(darkmod_dest, sizeof(darkmod_dest), "%s/textures/"	   , output_dir);

	printf("Moving contents of %s to %s\n", darkmod_src, darkmod_dest);

	DIR* darkmod_dir = opendir(darkmod_src);

	while ((entry = readdir(darkmod_dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		char new_name[1024];
		snprintf(new_name, sizeof(new_name), "d_%s", entry->d_name);

		char old_path[2048];
		char new_path[2048];

		snprintf(old_path, sizeof(old_path), "%s/%s", darkmod_src , entry->d_name);
		snprintf(new_path, sizeof(new_path), "%s/%s", darkmod_dest, new_name);

		// wont work on windows im sure
		if (rename(old_path, new_path) == 0) {
			printf("Moved: %s -> %s\n", old_path, new_path);
		} else {
			perror("Failed to move directory");
		}
	}

	#ifdef _WIN32
	_rmdir(darkmod_src);
	#else
	rmdir(darkmod_src);
	#endif

	closedir(darkmod_dir);

	// flatten directories
	
	char tex_dir_str[1024];
	snprintf(tex_dir_str, sizeof(tex_dir_str), "%s/textures/", output_dir);

	DIR* tex_dir = opendir(tex_dir_str);

	while ((entry = readdir(tex_dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		
		char first_folder[path_max];
		snprintf(first_folder, sizeof(first_folder), "%s/%s", tex_dir_str, entry->d_name);
		
		if (!is_directory(first_folder))
			continue;
		
		printf("Processing: %s\n", first_folder);
		
		flatten_recursive(first_folder, first_folder, "");
		remove_empty_dirs(first_folder);
	}
	
	closedir(tex_dir);

	// handle normalmaps
	
        char normal_dir_str[1024];
        snprintf(normal_dir_str, sizeof(normal_dir_str), "%s/textures/d_normal/", output_dir);

	ensure_directory(normal_dir_str);

	DIR* normal_dir = opendir(tex_dir_str);

	while ((entry = readdir(normal_dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (strcmp(entry->d_name, "d_normal") == 0)
			continue;
	
		char folder_path[path_max];
		snprintf(folder_path, sizeof(folder_path), "%s/%s", tex_dir_str, entry->d_name);
		
		if (!is_directory(folder_path))
			continue;
	
		scan_and_move(tex_dir_str, folder_path, entry->d_name, "d_normal");
	}

	closedir(normal_dir);

	// rename everything else in d_ to t_
	
	char t_path[path_max];
	snprintf(t_path, sizeof(t_path), "%s/textures", output_dir);
	rename_non_d_folders(t_path);

	// program cleanup
	
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

int is_directory(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return 0;
	return S_ISDIR(st.st_mode);
}

int is_file(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return 0;
	return S_ISREG(st.st_mode);
}

void remove_empty_dirs(const char *path) {
	struct dirent *entry;
	DIR *dir = opendir(path);
	if (!dir) return;
	
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		continue;
	
		char child_path[path_max];
		snprintf(child_path, sizeof(child_path), "%s/%s", path, entry->d_name);
	
		if (is_directory(child_path)) {
			remove_empty_dirs(child_path);
			rmdir(child_path);  // succeeds only if empty
		}
	}
	closedir(dir);
}

void flatten_recursive(const char *dest_path, const char *src_path, const char *prefix) {
	struct dirent *entry;
	DIR *dir = opendir(src_path);
	if (!dir) {
		perror("opendir");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		continue;

		char full_path[path_max];
		snprintf(full_path, sizeof(full_path), "%s/%s", src_path, entry->d_name);

		if (is_directory(full_path)) {
			char new_prefix[path_max];
			if (prefix[0] == '\0') {
				snprintf(new_prefix, sizeof(new_prefix), "%s", entry->d_name);
			} else {
				snprintf(new_prefix, sizeof(new_prefix), "%s_%s", prefix, entry->d_name);
			}
			flatten_recursive(dest_path, full_path, new_prefix);
		
		} else if (is_file(full_path)) {
			// files directly in the first-level folder (empty prefix) are left alone
			if (prefix[0] == '\0')
			continue;
		
			char new_name[path_max];
			snprintf(new_name, sizeof(new_name), "%s_%s", prefix, entry->d_name);
		
			char target_path[path_max];
			snprintf(target_path, sizeof(target_path), "%s/%s", dest_path, new_name);
		
			if (rename(full_path, target_path) == 0) {
				printf("Moved: %s -> %s\n", full_path, target_path);
			} else {
				perror("rename failed");
			}
		}
	}
	closedir(dir);
}

void scan_and_move(const char *textures_path, const char *current_path,
                   const char *folder_name, const char *dest_dir_name)
{
	struct dirent *entry;
	DIR *dir = opendir(current_path);
	if (!dir) {
		perror("opendir");
		return;
	}
	
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		
		char full_path[path_max];
		snprintf(full_path, sizeof(full_path), "%s/%s", current_path, entry->d_name);
		
		if (is_directory(full_path)) {
			scan_and_move(textures_path, full_path, folder_name, dest_dir_name);
		} else if (is_file(full_path)) {
			if (strstr(entry->d_name, "local") != NULL || 
				strstr(entry->d_name, "normal") != NULL ||
				strstr(entry->d_name, "_n.") != NULL) {
		
				char new_name[path_max];
				snprintf(new_name, sizeof(new_name), "%s_%s", folder_name, entry->d_name);
				
				char dest_path[path_max];
				snprintf(dest_path, sizeof(dest_path), "%s/%s/%s",
				textures_path, dest_dir_name, new_name);
					
				if (rename(full_path, dest_path) == 0) {
					printf("Moved(normal): %s -> %s\n", full_path, dest_path);
				} else {
					perror("rename failed");
				}
			}
		}
	}
	closedir(dir);
}

void rename_non_d_folders(const char *textures_path)
{
    struct dirent *entry;
    DIR *dir = opendir(textures_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char old_path[path_max];
        snprintf(old_path, sizeof(old_path), "%s/%s", textures_path, entry->d_name);

        if (!is_directory(old_path))
            continue;

        // Leave d_ folders untouched
        if (strncmp(entry->d_name, "d_", 2) == 0)
            continue;

        // Skip if already t_ to avoid t_t_ double-prefix on re-runs
        if (strncmp(entry->d_name, "t_", 2) == 0)
            continue;

        char new_name[path_max];
        char new_path[path_max];

        snprintf(new_name, sizeof(new_name), "t_%s", entry->d_name);
        snprintf(new_path, sizeof(new_path), "%s/%s", textures_path, new_name);

        if (rename(old_path, new_path) == 0) {
            printf("Renamed: %s -> %s\n", old_path, new_path);
        } else {
            perror("rename failed");
        }
    }

    closedir(dir);
}

#ifdef _WIN32
void ensure_directory(const char *path) {
	if (!is_directory(path)) _mkdir(path);
}
#else
void ensure_directory(const char *path) {
	if (!is_directory(path)) mkdir(path, 0755);
}
#endif
