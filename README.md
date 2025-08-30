# Image
Tools for manipulating images.

How to assemble and run for Windows:
1. Download Image repository from GitHub
1. Download Console from https://github.com/Alex-byte-ai/Console place it next to Image repository
1. Download Utilities from https://github.com/Alex-byte-ai/Utilities place it next to Image repository
1. Download MinGW from
https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-19.1.1-12.0.0-ucrt-r2/winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2.7z
1. Unzip, rename folder to MinGW, move it to C:\\
	* Press Win + S and search for "Environment Variables."
	* Select Edit the system environment variables.
	* In the System Properties window, click Environment Variables....
	* Locate the PATH variable under System Variables, and click Edit.
	* Add C:\MinGW\bin
	* Add absolute path to Image\tasks\Debug\bin
2. Download and install Code::Blocks from https://www.codeblocks.org/downloads/binaries/
	* Open Code::Blocks, Settings/Compiler.../Toolchain executables/Auto-detect (Auto-detect works, if MinGW folder is on disk C:\\)
	* Settings/Editor.../Encoding: `UTF8`, `As default encoding`, `if conversion fails...`
	* Close Code::Blocks
2. Download zlib from https://zlib.net/zlib131.zip
2. Unzip, rename folder to Zlib-1.3.1, place this folder next to Image repository
2. Download cmake from https://cmake.org/download/ and launch it
	* Create Zlib-1.3.1-additional next to Image repository
	* Launch cmake
	* Where is the source code: Zlib-1.3.1
	* Where to build binaries: Zlib-1.3.1-additional
	* Configure
	* Generate
	* Remove everything in Zlib-1.3.1-additional, except libzlibstatic.a and zconf.h
3. Image/build.bat
3. Console/build.bat
3. Image/console.bat
3. Image/launch.bat
