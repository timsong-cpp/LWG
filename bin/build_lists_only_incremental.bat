@echo off
setlocal enabledelayedexpansion
call git status --porcelain > mailing\git.status
set STATUS_SIZE=0
for /f %%i in ("mailing\git.status") do set STATUS_SIZE=%%~zi

if %STATUS_SIZE% equ 0 (
    rem Working directory clean
    call git rev-parse HEAD > mailing\commit.new
) else (
    echo Working directory is dirty! Later incremental update will not work.
    del mailing\commit.new mailing\commit 2>nul
)

del mailing\git.status
if exist mailing\commit.prev (
   for /f %%i in (mailing\commit.prev) do set PREV_COMMIT=%%i
   call git diff --name-only !PREV_COMMIT! > mailing\changed_files.txt
   call git ls-files -o --exclude-standard xml/ >> mailing\changed_files.txt rem add any untracked files in xml/ to the list of changed files
   set PREV_COMMIT=
) else (
   del mailing\changed_files.txt 2>nul
)

bin\lists

if errorlevel 1 goto error
mailing\lwg-active.html
if exist mailing\commit.new (
    move /y mailing\commit.new mailing\commit
)

goto done

:error
del mailing\changed_files.txt mailing\updated_files.txt mailing\commit.new 2>nul
echo ***********************************
echo ********** build failure **********
echo ***********************************

:done
set STATUS_SIZE=
