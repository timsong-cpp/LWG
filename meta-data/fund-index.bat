@echo off
if %1$==$ goto error
if %2$==$ goto error
grep "<header>" %1 | sed -r --file=fund-index.sed >%2
echo Complete. See %2
goto ok

:error
echo Invoke: build_index html-input-file text-output-file

:ok
