# platform.mk

MACHINE=${strip ${shell ${CC} -dumpmachine}}
ifndef PLATFORM
	PLATFORM=UNKNOWN
	ifneq (, ${findstring -linux, ${MACHINE}})
		PLATFORM=LINUX
	endif
	ifneq (, ${findstring -freebsd, ${MACHINE}})
		PLATFORM=BSD
	endif
	ifneq (, ${findstring -mingw32, ${MACHINE}})
		PLATFORM=WINDOWS
	endif
	ifneq (, ${findstring -darwin, ${MACHINE}})
		PLATFORM=DARWIN
	endif
endif

ifeq (${PLATFORM}, LINUX)
	LIBS+=-lpthread
	FLAGS+=-DNPROC=`nproc` -DCACHELINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`
endif

ifeq (${PLATFORM}, WINDOWS)
	LIBS+=
	FLAGS+=-DNPROC=${NUMBER_OF_PROCESSORS}
endif

ifeq (${PLATFORM}, DARWIN)
	LIBS+=-lpthread
	FLAGS+=-Wno-missing-braces -DNPROC=`sysctl -n hw.logicalcpu` -DCACHELINESIZE=`sysctl -n hw.cachelinesize`
endif
