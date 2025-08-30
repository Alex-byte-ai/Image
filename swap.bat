@echo off
setlocal

if exist "-tasks" (
    ren "-tasks" "--tasks"
    if exist "tasks" (
        ren "tasks" "-tasks"
    )
    ren "--tasks" "tasks"
) else (
    if exist "tasks" (
        xcopy "tasks" "-tasks" /E /I /H /K /Y
    )
)

endlocal