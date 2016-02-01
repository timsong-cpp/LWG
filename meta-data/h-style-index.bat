@echo off
if %1$==$ goto error
if %2$==$ goto error
if %3$==$ goto error
grep "<h[1-9]" %2 | sed -r --file=h-style-index.sed | grep "section-prefix" | sed -r "s/section-prefix/%1/g" >%3
echo Complete. See %3
echo Note that headings split between multiple lines are not handled correctly
echo and first must be edited down to a single line.
goto ok

:error
echo Invoke: h-style-index section-prefix html-input-file text-output-file
echo Example: h-style-index filesys.ts n4099.html temp.txt

:ok
