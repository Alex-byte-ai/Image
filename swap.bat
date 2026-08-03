@echo off
setlocal

if exist "-tasks" (
    ren "-tasks" "--tasks"
    if exist "tasks" (
        ren "tasks" "-tasks"
    )
    ren "--tasks" "tasks"
)

endlocal