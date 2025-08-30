set CodeBlocks="C:\Program Files\CodeBlocks\codeblocks.exe"

start "" /D "." %CodeBlocks% --build --target="Debug" Tasks.cbp
start "" /D "." %CodeBlocks% --build --target="Release" Tasks.cbp
exit
