[Setup]

#define AppName "Agros2D"
#define AppVersion "3.0.0.4514"
#define AppDate "2013-08-14"
#define OutputBaseFileName "Agros2D" + "_" + AppVersion

AppName={#AppName}
AppVerName={#AppName} {#AppVersion} ({#AppDate})
OutputBaseFilename={#OutputBaseFileName}
DefaultGroupName=Agros2D
LicenseFile=COPYING

AppId=Agros2D
AppPublisher=hpfem.org
AppCopyright=hpfem.org
AppPublisherURL=http://www.agros2d.org/
AppMutex=Agros2D
OutputDir=setup
DefaultDirName={pf}\Agros2D
UninstallDisplayIcon={app}\images\agros2d.ico
Compression=lzma/max
PrivilegesRequired=admin

WizardImageFile=resources_source\images\setup\SetupModern.bmp
WizardSmallImageFile=resources_source\images\setup\SetupModernSmall.bmp

[Languages]

[Files]
Source: agros2d_util.dll; DestDir: {app}; DestName: agros2d_util.dll
Source: agros2d_pythonlab_library.dll; DestDir: {app}; DestName: agros2d_pythonlab_library.dll
Source: agros_pythonlab.exe; DestDir: {app}; DestName: PythonLab.exe
Source: agros2d_library.dll; DestDir: {app}; DestName: agros2d_library.dll
Source: libs\*.dll; DestDir: {app}/libs
Source: agros2d.exe; DestDir: {app}; DestName: Agros2D.exe
Source: agros2d_solver.exe; DestDir: {app}; DestName: Solver.exe
;Source: agros2d_remote.exe; DestDir: {app}; DestName: Remote.exe
Source: resources\images\agros2d.ico; DestDir: {app}; DestName: Agros2D.ico
Source: resources\*; DestDir: {app}/resources; Flags: recursesubdirs
Source: ..\agros2d-win32\*; DestDir: {app}; Flags: recursesubdirs

[Icons]
Name: {group}\Agros2D; Filename: {app}\Agros2D.exe; WorkingDir: {app}
Name: {group}\Web pages; Filename: {app}\Agros2D.url
Name: {group}\COPYING; Filename: {app}\COPYING
Name: {group}\Uninstall; Filename: {uninstallexe}
Name: {commondesktop}\Agros2D; Filename: {app}\Agros2D.exe; WorkingDir: {app}; Tasks: desktopicon

[Tasks]
Name: desktopicon; Description: Create icon on desktop

[Run]
;

[Registry]
; a2d
Root: HKCR; SubKey: .a2d; ValueType: string; ValueData: Agros2D.Data; Flags: uninsdeletekey
Root: HKCR; SubKey: Agros2D.Data; ValueType: string; ValueData: Agros2D data file; Flags: uninsdeletekey
Root: HKCR; SubKey: Agros2D.Data\Shell\Open\Command; ValueType: string; ValueData: """{app}\Agros2D.exe"" ""%1"""; Flags: uninsdeletevalue
Root: HKCR; Subkey: Agros2D.Data\DefaultIcon; ValueType: string; ValueData: {app}\Agros2D.ico; Flags: uninsdeletevalue

[INI]
Filename: {app}\Agros2D.url; Section: InternetShortcut; Key: URL; String: http://www.hpfem.org/agros2d

[UninstallDelete]
Type: files; Name: {app}\Agros2D.url

[Code]

