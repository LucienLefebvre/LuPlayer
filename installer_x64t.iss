; -- 64Bit.iss --
; Demonstrates installation of a program built for the x64 (a.k.a. AMD64)
; architecture.
; To successfully run this installation and the program it installs,
; you must have a "x64" edition of Windows.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
OutputBaseFilename=Multiplayer_Beta_V0.9.1.1_x64_setup
AppName=Multiplayer
AppVersion=0.9.1.1
WizardStyle=modern
DefaultDirName={autopf}\Multiplayer
DefaultGroupName=Multiplayer
UninstallDisplayIcon={app}\Multiplayer.exe
Compression=lzma2
SolidCompression=yes
OutputDir=userdocs:Inno Setup
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "Multiplayer.exe"; DestDir: "{app}"; DestName: "Multiplayer.exe"
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme
Source : "ffmpeg.exe"; DestDir: "{app}"
Source : "ffplay.exe"; DestDir: "{app}"
Source : "ffprobe.exe"; DestDir: "{app}"
Source : "avcodec-58.dll"; DestDir: "{app}"
Source : "avdevice-58.dll"; DestDir: "{app}"
Source : "avfilter-7.dll"; DestDir: "{app}"
Source : "avformat-58.dll"; DestDir: "{app}"
Source : "avutil-56.dll"; DestDir: "{app}"
Source : "swresample-3.dll"; DestDir: "{app}"
Source : "swscale-5.dll"; DestDir: "{app}"  
Source : "postproc-55.dll"; DestDir: "{app}"  
Source : "lame.exe"; DestDir: "{app}"
Source : "Multiplayer.touchosc"; DestDir: "{app}"
Source : "Documentation\Doc.pdf"; DestDir: "{app}\Documentation"
[Dirs]
Name: "{userappdata}\Multiplayer\Saves"
[Icons]
Name: "{group}\Multiplayer"; Filename: "{app}\Multiplayer.exe"
[Registry]
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\CapabilityAccessManager\ConsentStore\microphone"; ValueType: string; ValueName: "Value"; ValueData: "Allow"