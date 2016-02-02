@echo off
rem Prerequisite: POSIX grep and sed, such as from Cygwin
if %1$==$ goto error
if %2$==$ goto error
if %3$==$ goto error
grep "<h[1-9]" %2 | sed -r --file=h-style-index.sed | grep "section-prefix" | sed -r "s/section-prefix/%1/g" >%3
echo Complete. See %3
echo TODO: Handle headings split across two lines. Filesystem TS needs that.
goto ok

:error
echo Invoke: h-style-index section-prefix html-input-file text-output-file
echo Example: h-style-index filesys.ts n4099.html temp.txt

:ok
