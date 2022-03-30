.PHONY: build
.PHONY: clean
	
###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. make PLATFORM=trimui)
endif
ifeq (,$(CROSS_COMPILE))
$(error missing CROSS_COMPILE for this toolchain)
endif

###########################################################

CC = $(CROSS_COMPILE)gcc
SYSROOT := $(shell $(CC) --print-sysroot)
INCLUDEDIR = $(SYSROOT)/usr/include
CFLAGS = -I$(INCLUDEDIR) -I. -I../common -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z) -std=gnu99
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lmsettings -lrt -lz -lm
OPTM=-O3

build:
	$(CC) -o MiniUI main.c ../common/common.c $(CFLAGS) $(LDFLAGS) $(OPTM) -ldl -rdynamic
clean:
	rm -f MiniUI