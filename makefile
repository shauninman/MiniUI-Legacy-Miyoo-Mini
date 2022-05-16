.PHONY: clean

###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. PLATFORM=trimui make)
endif

###########################################################

BUILD_ARCH!=uname -m
BUILD_HASH!=git rev-parse --short HEAD
BUILD_TIME!=date "+%Y-%m-%d %H:%M:%S"
BUILD_REPO=https://github.com/shauninman/MiniUI
BUILD_GCC:=$(shell $(CROSS_COMPILE)gcc -dumpfullversion -dumpversion)

RELEASE_TIME!=date +%Y%m%d
RELEASE_BASE=MiniUI-$(RELEASE_TIME)
RELEASE_DOT!=find ./releases/. -regex ".*/$(RELEASE_BASE)-[0-9]+\.zip" -printf '.' | wc -m
RELEASE_NAME=$(RELEASE_BASE)-$(RELEASE_DOT)
EXTRAS_NAME=$(RELEASE_NAME)-extras

LIBC_LIB=/opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib
BUNDLE_LIBS=

GCC_VER_GTE9_0 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \>= 9.0 | bc )
ifeq "$(GCC_VER_GTE9_0)" "1"
  BUNDLE_LIBS=bundle
endif

all: lib sdl core emu payload $(BUNDLE_LIBS) zip

extras: emu

lib:
	cd ./src/libmsettings && make
	cd ./src/libmmenu && make

sdl:
	cd ./third-party/SDL-1.2 && ./make.sh

core:
	cd ./src/batmon && make
	cd ./src/keymon && make
	cd ./src/lumon && make
	cd ./src/progressui && make
	cd ./src/miniui && make
	cd ./src/show && make
	cd ./src/confirm && make
	cd ./src/say && make
	cd ./src/blank && make

emu:
	cd ./third-party/picoarch && make platform=miyoomini -j
	./bits/cores.sh > ./cores.txt

tools:
	cd ./third-party/DinguxCommander && make -j
	cd ./third-party/screenshot && make
	cd ./third-party/logotweak/logomake && make
	cd ./third-party/logotweak/logowrite && make

payload:
	rm -rf ./build
	mkdir -p ./releases
	mkdir -p ./build
	cp -R ./skeleton/. ./build/PAYLOAD
	cp -R ./extras/. ./build/EXTRAS
	fmt -w 40 -s ./skeleton//README.txt > ./build/PAYLOAD/README.txt
	fmt -w 40 -s ./extras//README.txt > ./build/EXTRAS/README.txt
	mv ./build/PAYLOAD/miyoo/app/keymon.sh ./build/PAYLOAD/miyoo/app/keymon
	cd ./build && find . -type f -name '.keep' -delete
	cd ./build && find . -type f -name '.DS_Store' -delete
	cp ./src/libmsettings/libmsettings.so ./build/PAYLOAD/.system/lib/
	cp ./src/libmmenu/libmmenu.so ./build/PAYLOAD/.system/lib/
	cp ./third-party/SDL-1.2/build/.libs/libSDL-1.2.so.0.11.5 ./build/PAYLOAD/.system/lib/libSDL-1.2.so.0
	cp ./src/batmon/batmon ./build/PAYLOAD/.system/bin/
	cp ./src/keymon/keymon ./build/PAYLOAD/.system/bin/
	cp ./src/lumon/lumon ./build/PAYLOAD/.system/bin/
	cp ./src/progressui/progressui ./build/PAYLOAD/.system/bin/
	cp ./src/progressui/progress.sh ./build/PAYLOAD/.system/bin/progress
	cp ./src/miniui/MiniUI ./build/PAYLOAD/.system/paks/MiniUI.pak/
	cp ./src/show/show ./build/PAYLOAD/.system/bin/
	cp ./src/confirm/confirm ./build/PAYLOAD/.system/bin/
	cp ./src/say/say ./build/PAYLOAD/.system/bin/
	cp ./src/blank/blank ./build/PAYLOAD/.system/bin/
	cp ./src/say/say ./build/PAYLOAD/miyoo/app/
	cp ./src/blank/blank ./build/PAYLOAD/miyoo/app/
	cp ./third-party/picoarch/picoarch ./build/PAYLOAD/.system/bin/
	cp ./third-party/picoarch/fceumm_libretro.so ./build/PAYLOAD/.system/cores/
	cp ./third-party/picoarch/gambatte_libretro.so ./build/PAYLOAD/.system/cores/
	cp ./third-party/picoarch/gpsp_libretro.so ./build/PAYLOAD/.system/cores/
	cp ./third-party/picoarch/pcsx_rearmed_libretro.so ./build/PAYLOAD/.system/cores/
	cp ./third-party/picoarch/picodrive_libretro.so ./build/PAYLOAD/.system/cores/
	cp ./third-party/picoarch/snes9x2005_plus_libretro.so ./build/PAYLOAD/.system/cores/
	cp ./third-party/DinguxCommander/output/DinguxCommander ./build/EXTRAS/Tools/Files.pak/
	cp -r ./third-party/DinguxCommander/res ./build/EXTRAS/Tools/Files.pak/
	cp ./third-party/screenshot/screenshot ./build/EXTRAS/Tools/Screenshots.pak/
	cp ./third-party/picoarch/beetle-pce-fast_libretro.so ./build/EXTRAS/Emus/PCE.pak/mednafen_pce_fast_libretro.so
	cp ./third-party/picoarch/pokemini_libretro.so ./build/EXTRAS/Emus/PKM.pak/
	# cp ./third-party/picoarch/genesis-plus-gx/genesis_plus_gx_libretro.so ./build/EXTRAS/Emus/MCD.pak/
	# cp -R ./bits/bootlogos/pak/. ./build/EXTRAS/Tools/Single-use/bootlogo.tmp
	# cp ./third-party/logotweak/logomake/logomake ./build/EXTRAS/Tools/Single-use/bootlogo.tmp/
	# cp ./third-party/logotweak/logowrite/logowrite ./build/EXTRAS/Tools/Single-use/bootlogo.tmp/
	# cd ./build/EXTRAS/Tools/Single-use/ && cp -R ./bootlogo.tmp/. "02) Remove MiniUI Boot Logo.pak"
	# cp -R ./bits/bootlogos/miniui/. ./build/EXTRAS/Tools/Single-use/bootlogo.tmp/
	# cd ./build/EXTRAS/Tools/Single-use/ && cp -R ./bootlogo.tmp/. "02) Add MiniUI Boot Logo.pak"
	# rm -rf ./build/EXTRAS/Tools/Single-use/bootlogo.tmp
	rm -rf ./build/EXTRAS/Emus/MCD.pak

bundle:
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/ld-linux-armhf.so.3 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libc.so.6 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libcrypt.so.1 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libdl.so.2 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libgcc_s.so.1 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libm.so.6 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libpcprofile.so ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libpthread.so.0 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libresolv.so.2 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/librt.so.1 ./build/PAYLOAD/.system/lib/
	cp -L /opt/miyoomini-toolchain/arm-none-linux-gnueabihf/libc/lib/libstdc++.so.6 ./build/PAYLOAD/.system/lib/

zip:
	cd ./build/PAYLOAD/.system/paks/MiniUI.pak && echo "$(RELEASE_NAME).zip\n$(BUILD_HASH)" > version.txt
	cp ./cores.txt ./build/PAYLOAD/.system/paks/MiniUI.pak
	cd ./build/PAYLOAD && zip -r MiniUI.zip .system .tmp_update
	mv ./build/PAYLOAD/MiniUI.zip ./build/PAYLOAD/miyoo/app/
	cd ./build/PAYLOAD && zip -r ../../releases/$(RELEASE_NAME).zip Bios Roms Saves miyoo README.txt
	cd ./build/EXTRAS && zip -r ../../releases/$(EXTRAS_NAME).zip Bios Emus Roms Saves Tools README.txt
	echo "$(RELEASE_NAME)" > ./build/latest.txt

rezip: payload $(BUNDLE_LIBS) zip
	
clean:
	rm -rf ./build
	cd ./src/libmsettings && make clean
	cd ./src/libmmenu && make clean
	cd ./third-party/SDL-1.2 && make distclean
	cd ./src/batmon && make clean
	cd ./src/keymon && make clean
	cd ./src/lumon && make clean
	cd ./src/progressui && make clean
	cd ./src/miniui && make clean
	cd ./src/show && make clean
	cd ./src/confirm && make clean
	cd ./src/say && make clean
	cd ./src/blank && make clean
	cd ./third-party/picoarch && make platform=miyoomini clean
	cd ./third-party/DinguxCommander && make clean