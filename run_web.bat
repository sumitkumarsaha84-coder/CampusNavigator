@echo off
setlocal enabledelayedexpansion
title Campus Navigator - Web Dashboard Launcher
cd /d "%~dp0"

echo.
echo ================================================================================
echo    CAMPUS NAVIGATOR - SMART CAMPUS ROUTE FINDER
echo    Web Dashboard + C++17 REST API Launcher
echo ================================================================================
echo.

:: Set GCC toolchain path
set "GCC_BIN=C:\Users\sumit\.gemini\antigravity\scratch\w64devkit\bin"
if not exist "%GCC_BIN%\g++.exe" (
    echo [ERROR] GCC not found at: %GCC_BIN%
    echo         Please update GCC_BIN in this script to your g++.exe directory.
    pause
    exit /b 1
)
set "PATH=%GCC_BIN%;%PATH%"

:: Compile the server binary
echo [BUILD] Compiling CampusNavigatorServer.exe ...
g++ -std=c++17 -Wall -O2 -Iinclude ^
    src/main.cpp ^
    src/CampusGraph.cpp ^
    src/PathFinder.cpp ^
    src/CampusData.cpp ^
    src/TerminalUI.cpp ^
    src/HttpServer.cpp ^
    -lws2_32 ^
    -o CampusNavigatorServer.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation FAILED. Fix errors above before launching.
    pause
    exit /b 1
)
echo [BUILD] Compilation successful.
echo.

:: Choose port (default 8080, accept override as first argument)
set PORT=8080
if not "%~1"=="" set PORT=%~1

echo [INFO]  Dashboard URL : http://localhost:%PORT%
echo [INFO]  API Base URL  : http://localhost:%PORT%/api
echo [INFO]  Press Ctrl+C to stop the server.
echo.

:: Open the browser
start "" "http://localhost:%PORT%"

:: Start the server (blocking - keeps window alive)
CampusNavigatorServer.exe --web --port %PORT% --webroot web
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Server exited with error code %ERRORLEVEL%
    echo         Port %PORT% may already be in use. Try: run_web.bat 8081
    pause
)
endlocal
