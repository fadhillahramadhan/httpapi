@echo off
echo Building HttpApi Framework...
echo =============================

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Navigate to build directory
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo To run the example server:
echo   cd build
echo   .\Release\httpapi_app.exe
echo.
echo The server will start on http://127.0.0.1:3000
echo.
pause 