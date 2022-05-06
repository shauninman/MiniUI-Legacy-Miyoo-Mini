TARGET = logowrite

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -O3 -marm -mtune=cortex-a7 -march=armv7ve+simd -mfpu=neon-vfpv4 -mfloat-abi=hard -Wall
LDFLAGS = -s

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)

install:
	echo do nothing for install
