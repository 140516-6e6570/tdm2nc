#define MATE_IMPLEMENTATION // Adds the implementation of functions for mate
#include "./thirdparty/mate.h/mate.h"

int main(void) {
	StartBuild();
	{
		FlagBuilder flag_builder = FlagBuilderCreate();
		
		Executable e = CreateExecutable((ExecutableOptions){
			.output	  = "tdm2nc",
			.warnings = FLAG_WARNINGS,
			#ifdef _WIN32
			.flags	  = "-DZIP_HAVE_SYMLINK=0",
			#else
			.flags	  = "-DZIP_HAVE_SYMLINK=1"
			#endif
		});

		AddIncludePaths(e, "./thirdparty/zip/src/");

		AddFile(e, "./src/*.c");
		AddFile(e, "./thirdparty/zip/src/zip.c");

		if (isWindows()) {
			AddIncludePaths(e, "./thirdparty/dirent/include/");
		}

		InstallExecutable(e);

	}
	EndBuild();
}
