#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BUTTON_A KEY_SPACE
#define BUTTON_B KEY_LEFTCTRL

int main(int argc , char* argv[]) {
	if (argc<1) {
		puts("Usage: confirm [any]");
		puts("  A = confirm B = cancel, unless 'any' arg is provided, then any button confirms");
		return 0;
	}
	int any = argc>1;
	
	int confirmed = 0; // true in shell

	int input_fd = open("/dev/input/event0", O_RDONLY);
	struct input_event	event;
	while (read(input_fd, &event, sizeof(event))==sizeof(event)) {
		if (event.type!=EV_KEY || event.value>1) continue;
		if (event.type==EV_KEY) {
			if (any || event.code==BUTTON_A) break;
			else if (event.code==BUTTON_B) {
				confirmed = 1; // false in shell terms
				break;
			}
		}
	}
	
	// clear screen
	int fb0_fd = open("/dev/fb0", O_RDWR);
	struct fb_var_screeninfo vinfo;
	ioctl(fb0_fd, FBIOGET_VSCREENINFO, &vinfo);
	int map_size = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel / 8); // 640x480x4
	char* fb0_map = (char*)mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb0_fd, 0);
	memset(fb0_map, 0, map_size);
	
	return confirmed;
}
