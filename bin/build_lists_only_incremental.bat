@echo off
setlocal enabledelayedexpansion
call git status --porcelain > mailing\git.status
set STATUS_SIZE=0
for /f %%i in ("mailing\git.status") do set STATUS_SIZE=%%~zi
if %STATUS_SIZE% equ 0 (
    rem Working directory clean
    call git rev-parse HEAD > mailing/commit.new
    del mailing\git.status
) else (
    echo Working directory is dirty! Exiting.
    echo For incremental builds to reliably work, please commit your changes.
    del mailing\git.status
    goto done
)

if exist mailing\commit (
   for /f %%i in (mailing\commit) do set PREV_COMMIT=%%i
   call git diff --name-only !PREV_COMMIT! > mailing\changed_files.txt
   set PREV_COMMIT=
)

bin\lists

if errorlevel 1 goto error
mailing\lwg-active.html
if exist mailing\commit (
    copy mailing\commit mailing\commit.prev
)
copy mailing\commit.new mailing\commit
del mailing\commit.new

goto done

:error
del mailing\changed_files.txt mailing\updated_files.txt
echo ***********************************
echo ********** build failure **********
echo ***********************************

:done
set STATUS_SIZE=
