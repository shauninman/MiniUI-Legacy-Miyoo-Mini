#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GOLD_TRIAD 0xd2,0xb4,0x6c
#define kTextBoxMaxRows 8
#define kTextLineLength 256
#define kLineHeight 48

#define kMiniUIFont "/mnt/SDCARD/.system/res/BPreplayBold-unhinted.otf"
#define kStockFont "/customer/app/BPreplayBold.otf"

static void blit(void* _dst, int dst_w, int dst_h, void* _src, int src_w, int src_h, int ox, int oy) {
	uint8_t* dst = (uint8_t*)_dst;
	uint8_t* src = (uint8_t*)_src;
	
	for (int y=0; y<src_h; y++) {
		uint8_t* dst_row = dst + (((((dst_h - 1 - oy) - y) * dst_w) - 1 - ox) * 4);
		uint8_t* src_row = src + ((y * src_w) * 4);
		for (int x=0; x<src_w; x++) {
			float a = *(src_row+3) / 255.0;
			if (a>0.1) {
				*(dst_row+0) = *(src_row+0) * a;
				*(dst_row+1) = *(src_row+1) * a;
				*(dst_row+2) = *(src_row+2) * a;
				*(dst_row+3) = 0xff;
			}
			dst_row -= 4;
			src_row += 4;
		}
	}
}

int main(int argc , char* argv[]) {
	if (argc<2) {
		puts("Usage: say \"message\"");
		return EXIT_SUCCESS;
	}
	
	char* path = NULL;
	if (access(kMiniUIFont, F_OK)==0) path = kMiniUIFont;
	else if (access(kStockFont, F_OK)==0) path = kStockFont;
	if (!path) return 0;
	
	char str[kTextBoxMaxRows*kTextLineLength];
	strncpy(str,argv[1],kTextBoxMaxRows*kTextLineLength);
	
	int fb0_fd = open("/dev/fb0", O_RDWR);
	struct fb_var_screeninfo vinfo;
	ioctl(fb0_fd, FBIOGET_VSCREENINFO, &vinfo);
	int map_size = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel / 8); // 640x480x4
	char* fb0_map = (char*)mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb0_fd, 0);

	TTF_Init();
	TTF_Font* font = TTF_OpenFont(path, 32);
	
	int ox = 0;
	int oy = 0;
	int width = 640;
	int height = 480;
	SDL_Color gold = {GOLD_TRIAD};
	SDL_Surface* text;
	char* rows[kTextBoxMaxRows];
	int row_count = 0;

	char* tmp;
	rows[row_count++] = str;
	while ((tmp=strchr(rows[row_count-1], '\n'))!=NULL) {
		if (row_count+1>=kTextBoxMaxRows) break;
		rows[row_count++] = tmp+1;
	}
	
	int rendered_height = kLineHeight * row_count;
	int y = oy;
	y += (height - rendered_height) / 2;
	
	char line[kTextLineLength];
	for (int i=0; i<row_count; i++) {
		int len;
		if (i+1<row_count) {
			len = rows[i+1]-rows[i]-1;
			if (len) strncpy(line, rows[i], len);
			line[len] = '\0';
		}
		else {
			len = strlen(rows[i]);
			strcpy(line, rows[i]);
		}
		
		if (len) {
			text = TTF_RenderUTF8_Blended(font, line, gold);
			int x = ox;
			x += (width - text->w) / 2;			
			blit(fb0_map,640,480,text->pixels,text->w,text->h,x,y);
			SDL_FreeSurface(text);
		}
		y += kLineHeight;
	}
	
	TTF_CloseFont(font);
	TTF_Quit();
	munmap(fb0_map, map_size);
	close(fb0_fd);
	
	return EXIT_SUCCESS;
}
