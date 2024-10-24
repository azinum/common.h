:: build.bat

@echo off

set CC=gcc
set LIBS=
set FLAGS=-Wall -O2 -I.. -DNPROC=%NUMBER_OF_PROCESSORS% -DVERBOSE

%CC% test_thread.c -o test_thread.exe %LIBS% %INC% %FLAGS%
%CC% test_thread_with_mutex.c -o test_thread_with_mutex.exe %LIBS% %INC% %FLAGS%
%CC% test_thread_sync.c -o test_thread_sync.exe %LIBS% %INC% %FLAGS%

test_thread.exe
test_thread_with_mutex.exe
test_thread_sync.exe
