[Setup]

AppName=Carbon 2D
AppVerName=Carbon 2D 0.7
OutputBaseFilename=Carbon2D_0.7.0
DefaultGroupName=Carbon 2D
LicenseFile=COPYING

AppId=Carbon2D
AppPublisher=hp-fem
AppCopyright=hp-fem
AppPublisherURL=http://www.hpfem.org
AppMutex=Carbon2D
OutputDir=setup
DefaultDirName={pf}\Carbon 2D
UninstallDisplayIcon={app}\images\carbon2d.ico
Compression=lzma/max
PrivilegesRequired=admin

WizardImageFile=install\win32\setup\SetupModern.bmp
WizardSmallImageFile=install\win32\setup\SetupModernSmall.bmp

[Languages]

[Files]
Source: release\carbon2d.exe; DestDir: {app}; DestName: Carbon2D.exe
Source: install\win32\triangle.exe; DestDir: {app}; DestName: triangle.exe
Source: COPYING; DestDir: {app}; DestName: COPYING
Source: README; DestDir: {app}; DestName: README
Source: data\*.h2d; DestDir: {app}/data
Source: lang\*.qm; DestDir: {app}/lang
Source: doc\html\*; DestDir: {app}/doc/html
Source: doc\html\style\*; DestDir: {app}/doc/html/style
Source: install\win32\*.dll; DestDir: {app}

[Icons]
Name: {group}\Carbon 2D; Filename: {app}\Carbon2D.exe; WorkingDir: {app}
Name: {group}\COPYING; Filename: {app}\COPYING
Name: {group}\Uninstall; Filename: {uninstallexe}
Name: {commondesktop}\Carbon 2D; Filename: {app}\Carbon2D.exe; WorkingDir: {app}; Tasks: desktopicon

[Tasks]
Name: desktopicon; Description: Create icon on desktop

[Run]
;

[Registry]
;

[INI]
;Filename: {app}\OverHead.url; Section: InternetShortcut; Key: URL; String: http://www.skeleton.cz/overhead

[UninstallDelete]
;Type: files; Name: {app}\OverHead.url

[Code]

