@echo off
setlocal enabledelayedexpansion
rem WARNING This batch file assumes a checkout of the gh-pages branch,
rem in a directory named issues-gh-pages, at the same level as the master branch checkout
if not exist mailing\commit (
   echo ERROR: missing mailing\commit
   echo Maybe you built the lists with a dirty working directory?
   goto end
)
if not exist mailing\updated_files.txt ( 
   echo ERROR: missing mailing\updated_files.txt
   goto end
)

pushd ..\issues-gh-pages
call git pull
popd

fc ..\issues-gh-pages\commit mailing\commit.prev > NUL

if errorlevel 1 (
   for /f %%i in (mailing\commit.prev) do set PREV_COMMIT_MAILING=%%i
   for /f %%i in (issues-gh-pages\commit) do set PREV_COMMIT_GH=%%i
   
   echo WARNING: Commit mismatch
   echo gh-pages branch is generated from !PREV_COMMIT_MAILING!
   echo update list is based on changes from commit !PREV_COMMIT_GH!
   echo COPYING EVERYTHING 
   set PREV_COMMIT_MAILING=
   set PREV_COMMIT_GH=
   goto update_all
)
find /c /v "" <mailing\updated_files.txt >linecount.txt
for /f %%i in (linecount.txt) do set LINECOUNT=%%i
del linecount.txt

if %LINECOUNT% gtr 300 (
   echo WARNING: Number of files to update is over 300
   echo COPYING EVERYTHING
   goto update_all
)
rem Perform incremental update
for /f %%i in (mailing\updated_files.txt) do (
   set "i=%%i"
   set "f=!i:/=\!"
   copy /y !f! ..\issues-gh-pages
)
copy /y mailing\commit ..\issues-gh-pages
copy /y mailing\updated_files.txt ..\issues-gh-pages
pushd ..\issues-gh-pages
for /f %%i in (updated_files.txt) do (
   set "i=%%i"
   set "f=!i:/=\!"
   for %%c in ("!f!") do call git add %%~nxc
)
set i=
set f=
del updated_files.txt
call git add commit
call git commit -m"Update"
call git push  "origin" gh-pages:gh-pages
popd
move /y mailing\commit mailing\commit.prev
copy /y mailing\lwg-toc.html mailing\lwg-toc.prev.html
goto end

:update_all
copy /y mailing\lwg-*.html ..\issues-gh-pages
copy /y mailing\unresolved-*.html ..\issues-gh-pages
copy /y mailing\votable-*.html ..\issues-gh-pages
copy /y mailing\issue*.html ..\issues-gh-pages >nul
copy /y mailing\commit ..\issues-gh-pages
pushd ..\issues-gh-pages
call git add issue*.html lwg-*.html unresolved-*.html votable-*.html commit
call git commit -m"Update"
call git push  "origin" gh-pages:gh-pages
popd
move /y mailing\commit mailing\commit.prev
copy /y mailing\lwg-toc.html mailing\lwg-toc.prev.html

:end
set LINECOUNT=
