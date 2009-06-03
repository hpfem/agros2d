[Setup]

AppName=Carbon 2D
AppVerName=Carbon 2D 1.0
OutputBaseFilename=Carbon2D_1.0.0
DefaultGroupName=Carbon 2D
LicenseFile=install\win32\COPYING.txt

AppId=Carbon2D
AppPublisher=hp-fem
AppCopyright=hp-fem
AppPublisherURL=http://www.hpfem.org
AppMutex=Carbon2D
OutputDir=setup
DefaultDirName={pf}\Carbon 2D
UninstallDisplayIcon={app}\carbon2d-gui.ico
Compression=lzma/max
PrivilegesRequired=admin

WizardImageFile=install\win32\setup\images\SetupModern19.bmp
WizardSmallImageFile=install\win32\setup\images\SetupModernSmall19.bmp

[Languages]

[Files]
Source: release\carbon2d-gui.exe; DestDir: {app}; DestName: Carbon2D.exe
Source: install\win32\triangle.exe; DestDir: {app}; DestName: triangle.exe
Source: install\win32\COPYING.txt; DestDir: {app}; DestName: COPYING
Source: images\*.png; DestDir: {app}/images
Source: data\*.h2d; DestDir: {app}/data
Source: lang\*.qm; DestDir: {app}/lang
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

