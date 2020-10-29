@echo off

::cl /c dictionary.c ini.c atom.c fmap.c registry.c
cl /nologo /c /DMEM_COPY=memcpy /DMEM_MOVE=memmove /DMEM_COMPARE=memcmp /DMEM_SET=memset /DNPX_DEBUG jsizzle.c
::cl  jszl.i
::cl /Fotest\test.obj /Fetest\test.exe test\test.c /link dictionary.obj ini.obj atom.obj fmap.obj registry.obj
cl /nologo /Fotest\test.obj /Fe"test\test.exe" /DDEBUG test\test.c /link /MACHINE:x86 jsizzle.obj
::del dict.obj ini.obj atom.obj fmap.obj test.obj registry.obj json.obj
::del dilly.obj
echo.-------------------------------------
call test\test
