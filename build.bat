:: build.bat

@echo off

set CC=gcc
set LIBS=
set INC=.
set FLAGS=-I%INC% -Wall -O2 -DNPROC=%NUMBER_OF_PROCESSORS%

%CC% test_empty.c -o test_empty.exe %LIBS% %INC% %FLAGS%

:: %CC% test.c -o test.exe %LIBS% %INC% %FLAGS%
