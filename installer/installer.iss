[Setup]
AppName=FronterMind Harness
AppVersion=0.1.0
AppPublisher=Fabrizio Franco
DefaultDirName={autopf}\FronterMind
DefaultGroupName=FronterMind
SetupIconFile=assets\icons\frontermind-icon_negative@256x256.ico
UninstallDisplayIcon={app}\FronterMind.exe
OutputDir=.
OutputBaseFilename=FronterMind-Setup-v0.1.0-win64
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest

[Tasks]
Name: "desktopicon"; Description: "Crear un icono en el &Escritorio"; GroupDescription: "Iconos adicionales:"

[Files]
Source: "dist-win\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\FronterMind Harness"; Filename: "{app}\FronterMind.exe"
Name: "{autodesktop}\FronterMind Harness"; Filename: "{app}\FronterMind.exe"; Tasks: desktopicon
Name: "{group}\Desinstalar FronterMind"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\FronterMind.exe"; Description: "Ejecutar FronterMind Harness ahora"; Flags: nowait postinstall skipifsilent
