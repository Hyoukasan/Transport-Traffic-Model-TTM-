@echo off
setlocal
echo Building TTM...
where make >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "MAKE_CMD=make"
) else (
    where mingw32-make >nul 2>&1
    if %ERRORLEVEL% equ 0 (
        set "MAKE_CMD=mingw32-make"
    ) else (
        echo Error: GNU make not found. Install "make" or use WSL.
        pause
        exit /b 1
    )
)
%MAKE_CMD%
if errorlevel 1 (
    echo Build failed with error %%ERRORLEVEL%%.
    pause
    exit /b %%ERRORLEVEL%%
)
echo Build complete.
if exist TTM (
    echo Running TTM...
    .\TTM
) else (
    echo Executable TTM not found.
    pause
    exit /b 1
)
pause
endlocal
