set CodeBlocks="C:\Program Files\CodeBlocks\codeblocks.exe"

start "" /D "." %CodeBlocks% --build --target="Debug" Image.cbp
start "" /D "." %CodeBlocks% --build --target="Release" Image.cbp
start "" /D "tasks\" build.bat
exit
