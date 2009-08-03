[Setup]

AppName=Agros2D
AppVerName=Agros2D 0.9
OutputBaseFilename=Agros2D_0.9.1
DefaultGroupName=Agros2D
LicenseFile=COPYING

AppId=Agros2D
AppPublisher=hp-fem
AppCopyright=hp-fem
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
Source: install\win32\triangle.exe; DestDir: {app}; DestName: triangle.exe
Source: COPYING; DestDir: {app}; DestName: COPYING
Source: README; DestDir: {app}; DestName: README
Source: data\*.a2d; DestDir: {app}/data
Source: data\scripts\*.qs; DestDir: {app}/data/scripts
Source: lang\*.qm; DestDir: {app}/lang
Source: doc\help\agros2d.qhc; DestDir: {app}/doc/help
Source: doc\help\agros2d.qch; DestDir: {app}/doc/help
Source: install\win32\*.dll; DestDir: {app}

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
;

[INI]
Filename: {app}\Agros2D.url; Section: InternetShortcut; Key: URL; String: http://www.hpfem.org/agros2d

[UninstallDelete]
Type: files; Name: {app}\Agros2D.url

[Code]

