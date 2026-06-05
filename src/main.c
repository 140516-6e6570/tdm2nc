#ifdef _WIN32
#include <direct.h>
#include "dirent.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <stdio.h>

#include "zip.h"

int on_extract_entry(const char *filename, void *arg);

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Command requires exactly one argument pointing to your 'The Dark Mod' install.\n");
		return 1;
	}

	const char *output_dir = "tdm.pk3dir";

	#ifdef _WIN32
	_mkdir(output_dir);
	#else
	mkdir(output_dir, 0755);
	#endif

	printf("Output directory: %s\n", output_dir);

	struct dirent *entry;
	DIR *dir = opendir(argv[1]);

	if (dir == NULL) {
		perror("Failed to open directory.");
		return 1;
	}

	// 3. Iterate through files
	while ((entry = readdir(dir)) != NULL) {
		// Skip directories
		if (entry->d_type != DT_REG) continue;
		// Check extension (.pk4)
		char *ext = strrchr(entry->d_name, '.');
		if (ext && strcmp(ext, ".pk4") == 0) {
		// Construct full input path
			char input_path[1024];
			snprintf(input_path, 
				sizeof(input_path), 
				"%s/%s", 
				argv[1], 
				entry->d_name);
			printf("Processing: %s\n", input_path);

			// Open the zip archive
			// Note: Assuming 'zip.h' follows standard conventions like libzip
			int arg = 2;
			zip_extract(input_path, output_dir, on_extract_entry, &arg);
		}
	}	

	closedir(dir);
	printf("Extraction complete.\n");

	return 0;
}

int on_extract_entry(const char *filename, void *arg) {
	static int i = 0;
	int n = *(int *)arg;
	printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

	return 0;
}
