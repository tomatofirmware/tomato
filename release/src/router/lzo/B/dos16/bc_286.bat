@echo // Copyright (C) 1996-2008 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 16-bit
@echo //   Borland C/C++ + Pharlap 286DOS-Extender
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set CC=bcc286 -ml -2
set CF=-O1 -d -w -w-rch -w-sig %CFI% -Iinclude\lzo
set LF=%BLIB%

%CC% %CF% -Isrc -c @b\src.rsp
@if errorlevel 1 goto error
tlib %BLIB% @b\dos16\bc.rsp
@if errorlevel 1 goto error

%CC% %CF% -Iexamples examples\dict.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -Iexamples -DWITH_TIMER examples\lzopack.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -Iexamples examples\precomp.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -Iexamples examples\precomp2.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -Iexamples examples\simple.c %LF%
@if errorlevel 1 goto error

%CC% %CF% -Ilzotest lzotest\lzotest.c %LF%
@if errorlevel 1 goto error

%CC% %CF% -Iminilzo minilzo\testmini.c minilzo\minilzo.c
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
