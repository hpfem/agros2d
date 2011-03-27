@echo off
:: For windows platform development
:: Windows script for html help building

set helpSourcePath=".\doc\source"
set webPath=".\doc\web"
   
echo Building Agros2D html help...

sphinx-build -b html %helpSourcePath% %webPath%

echo Finished.
