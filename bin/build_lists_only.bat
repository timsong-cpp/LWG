@echo off
if exist mailing\changed_files.txt (
   del mailing\changed_files.txt
)
bin\lists
if errorlevel 1 goto error
mailing\lwg-active.html
goto done

:error
echo ***********************************
echo ********** build failure **********
echo ***********************************

:done

