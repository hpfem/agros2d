[Setup]

#define AppName "Agros2D"
#define AppVersion "3.0.0.3577"
#define AppDate "2013-03-15"
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
UninstallDisplayIcon={app}\images\Agros2d.ico
Compression=lzma/max
PrivilegesRequired=admin

WizardImageFile=install\win32\setup\SetupModern.bmp
WizardSmallImageFile=install\win32\setup\SetupModernSmall.bmp

[Languages]

[Files]
Source: agros2d.exe; DestDir: {app}; DestName: Agros2D.exe
Source: agros2d_pythonlab.exe; DestDir: {app}; DestName: Agros2D_PythonLab.exe
Source: agros2d_solver.exe; DestDir: {app}; DestName: Agros2D_Solver.exe
Source: agros2d_hermes2d.dll; DestDir: {app}; DestName: agros2d_hermes2d.dll
Source: agros2d_library.dll; DestDir: {app}; DestName: agros2d_library.dll
Source: libs\*.dll; DestDir: {app}/libs
Source: resources\images\agros2d.ico; DestDir: {app}; DestName: Agros2D.ico
Source: COPYING; DestDir: {app}; DestName: COPYING
Source: README; DestDir: {app}; DestName: README
Source: README_Python.txt; DestDir: {app}; DestName: README_Python.txt
Source: functions_pythonlab.py; DestDir: {app}; DestName: functions_pythonlab.py
Source: functions_agros2d.py; DestDir: {app}; DestName: functions_agros2d.py
Source: data\test\*.py; DestDir: {app}/data/test
Source: resources\*; DestDir: {app}/resources; Flags: recursesubdirs
Source: DLLs\*; DestDir: {app}/DLLs; Flags: recursesubdirs
Source: Lib\*; DestDir: {app}/Lib; Flags: recursesubdirs
Source: tcl\*; DestDir: {app}/tcl; Flags: recursesubdirs
Source: *.dll; DestDir: {app};
Source: triangle.exe; DestDir: {app}; DestName: triangle.exe
Source: GMSH.exe; DestDir: {app}; DestName: GMSH.exe
Source: python.exe; DestDir: {app}; DestName: python.exe
Source: pythonw.exe; DestDir: {app}; DestName: pythonw.exe

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
