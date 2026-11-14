$ErrorActionPreference = "Stop"

Write-Host "==> [1/4] Cargando entorno de compilacion de Visual Studio (x64)..." -ForegroundColor Cyan
$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
Import-Module "$vsPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

# Cerrar instancia previa si estuviera abierta para no bloquear el .exe
Stop-Process -Name FronterMind -Force -ErrorAction SilentlyContinue

Write-Host "==> [2/4] Configurando y compilando Release con CMake + Ninja..." -ForegroundColor Cyan
cmake --preset windows-release

if (Test-Path "build-win-release\build.ninja") {
    (Get-Content "build-win-release\build.ninja") -replace 'build build\.ninja: RERUN_CMAKE', 'build build.ninja.disabled: RERUN_CMAKE' | Set-Content "build-win-release\build.ninja"
}

ninja -C build-win-release
if ($LASTEXITCODE -ne 0) {
    Write-Error "Fallo la compilacion en modo Release."
    exit $LASTEXITCODE
}

Write-Host "==> [3/4] Empaquetando DLLs de Qt6 y creando portable .zip..." -ForegroundColor Cyan
Remove-Item -Recurse -Force dist-win -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path dist-win | Out-Null

Copy-Item ".\build-win-release\FronterMind.exe" "dist-win\"
Copy-Item ".\build-win-release\*.dll" "dist-win\" -ErrorAction SilentlyContinue

C:\Qt\6.12.0\msvc2022_64\bin\windeployqt.exe --release --qmldir src/ui/qml "dist-win\FronterMind.exe"

$zipName = "FronterMind-v0.1.0-Windows-x64.zip"
Compress-Archive -Path "dist-win\*" -DestinationPath $zipName -Force
Write-Host "  -> Portable creado: $zipName" -ForegroundColor Green

Write-Host "==> [4/4] Generando instalador .exe con Inno Setup..." -ForegroundColor Cyan
$isccPaths = @(
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe"
)

$iscc = $isccPaths | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($iscc) {
    & $iscc "installer\installer.iss"
    Write-Host "✅ ¡Release de Windows completada con exito!" -ForegroundColor Green
} else {
    Write-Warning "No se encontro Inno Setup (ISCC.exe). Se genero el .zip portable, pero falta instalar Inno Setup para crear el .exe instalador (winget install -e --id JRSoftware.InnoSetup)."
}
