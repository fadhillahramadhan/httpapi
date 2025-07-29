# HttpApi Framework Build Script
# PowerShell version

Write-Host "Building HttpApi Framework..." -ForegroundColor Green
Write-Host "=============================" -ForegroundColor Green

# Create build directory if it doesn't exist
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Navigate to build directory
Set-Location build

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Yellow
try {
    cmake .. -G "Visual Studio 17 2022" -A x64
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        Read-Host "Press Enter to continue"
        exit 1
    }
} catch {
    Write-Host "CMake configuration failed: $_" -ForegroundColor Red
    Read-Host "Press Enter to continue"
    exit 1
}

# Build the project
Write-Host "Building project..." -ForegroundColor Yellow
try {
    cmake --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        Read-Host "Press Enter to continue"
        exit 1
    }
} catch {
    Write-Host "Build failed: $_" -ForegroundColor Red
    Read-Host "Press Enter to continue"
    exit 1
}

Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "To run the example server:" -ForegroundColor Cyan
Write-Host "  cd build" -ForegroundColor White
Write-Host "  .\Release\httpapi_app.exe" -ForegroundColor White
Write-Host ""
Write-Host "The server will start on http://127.0.0.1:3000" -ForegroundColor Cyan
Write-Host ""

Read-Host "Press Enter to continue" 