:: build.bat

@echo off

set CC=gcc
set LIBS=
set FLAGS=-Wall -O2 -I.. -DNPROC=%NUMBER_OF_PROCESSORS% -DVERBOSE

%CC% test_thread.c -o test_thread.exe %LIBS% %INC% %FLAGS%
%CC% test_thread_with_mutex.c -o test_thread_with_mutex.exe %LIBS% %INC% %FLAGS%
%CC% test_thread_sync.c -o test_thread_sync.exe %LIBS% %INC% %FLAGS%
%CC% test_arena.c -o test_arena.exe %LIBS% %INC% %FLAGS%
%CC% test_random.c -o test_random.exe %LIBS% %INC% %FLAGS%
%CC% test_timer.c -o test_timer.exe %LIBS% %INC% %FLAGS%
%CC% test_log.c -o test_log.exe %LIBS% %INC% %FLAGS%
%CC% test_glob.c -o test_glob.exe %LIBS% %INC% %FLAGS%

test_thread.exe
test_thread_with_mutex.exe
test_thread_sync.exe
test_arena.exe
test_random.exe
test_timer.exe
test_log.exe
test_glob.exe
