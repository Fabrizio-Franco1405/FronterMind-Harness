param(
    [Parameter(Mandatory=$true)]
    [ValidateSet("configure", "run")]
    [string]$Action
)

$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
Import-Module "$vsPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

Stop-Process -Name FronterMind -Force -ErrorAction SilentlyContinue

if ($Action -eq "configure") {
    Remove-Item -Recurse -Force build-win -ErrorAction SilentlyContinue
    cmake --preset windows-debug
    if (Test-Path "build-win\build.ninja") {
        (Get-Content "build-win\build.ninja") -replace 'build build\.ninja: RERUN_CMAKE', 'build build.ninja.disabled: RERUN_CMAKE' | Set-Content "build-win\build.ninja"
    }
}
elseif ($Action -eq "run") {
    if (Test-Path "build-win\build.ninja") {
        (Get-Content "build-win\build.ninja") -replace 'build build\.ninja: RERUN_CMAKE', 'build build.ninja.disabled: RERUN_CMAKE' | Set-Content "build-win\build.ninja"
    }
    ninja -C build-win
    if ($LASTEXITCODE -eq 0) {
        $env:PATH = "C:\Qt\6.12.0\msvc2022_64\bin;$PWD\build-win;$env:PATH"
        .\build-win\FronterMind.exe
    }
}
