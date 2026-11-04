@echo off
echo ====================================================
echo      Building OSI 7-Layer Simulation Project
echo ====================================================

if not exist bin mkdir bin

echo Compiling Server (src/server.c + src/layers.c)...
gcc -Wall -O2 src/server.c src/layers.c -o bin/server.exe -lws2_32
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile server.exe!
    pause
    exit /b %ERRORLEVEL%
)

echo Compiling Client (src/client.c + src/layers.c)...
gcc -Wall -O2 src/client.c src/layers.c -o bin/client.exe -lws2_32
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile client.exe!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ====================================================
echo      Build Successful! Executables located in bin/
echo      - bin/server.exe
echo      - bin/client.exe
echo ====================================================
echo.
pause
