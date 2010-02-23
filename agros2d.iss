[Setup]

AppName=Agros2D
AppVerName=Agros2D 1.0.0.308 (2010-02-22)
OutputBaseFilename=Agros2D_1.0.0_beta1
DefaultGroupName=Agros2D
LicenseFile=COPYING

AppId=Agros2D
AppPublisher=hpfem.org
AppCopyright=hpfem.org
AppPublisherURL=http://www.hpfem.org/agros2d
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
Source: release\Agros2d.exe; DestDir: {app}; DestName: Agros2D.exe
Source: images\agros2d.ico; DestDir: {app}; DestName: Agros2D.ico
Source: src-remote\release\Agros2d-remote.exe; DestDir: {app}; DestName: Agros2D-remote.exe
Source: install\win32\triangle.exe; DestDir: {app}; DestName: triangle.exe
Source: install\win32\ffmpeg.exe; DestDir: {app}; DestName: ffmpeg.exe
Source: COPYING; DestDir: {app}; DestName: COPYING
Source: README; DestDir: {app}; DestName: README
Source: functions.py; DestDir: {app}; DestName: functions.py
Source: data\*.a2d; DestDir: {app}/data
Source: data\scripts\*.py; DestDir: {app}/data/scripts
Source: lang\*.qm; DestDir: {app}/lang
Source: doc\help\Agros2D.qhc; DestDir: {app}/doc/help
Source: doc\help\Agros2D.qch; DestDir: {app}/doc/help
Source: doc\report\template\default.css; DestDir: {app}/doc/report/template
Source: doc\report\template\template.html; DestDir: {app}/doc/report/template
Source: install\win32\*.dll; DestDir: {app}
Source: install\win32\sqldrivers\*.dll; DestDir: {app}/sqldrivers

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

