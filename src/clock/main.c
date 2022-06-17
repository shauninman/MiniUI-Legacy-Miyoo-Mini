// loosely based on https://github.com/gameblabla/clock_sdl_app

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

int main(int argc , char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(0);
	
	SDL_Surface* screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	
	#define RES_PATH "/mnt/SDCARD/.system/res/"
	SDL_Surface* confirm = IMG_Load(RES_PATH "confirm.png");
	SDL_Surface* digits = IMG_Load(RES_PATH "digits.png"); // 20x32
	
	SDL_Event event;
	int quit = 0;
	int save_changes = 0;
	int select_cursor = 0;
	
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	uint32_t day_selected = tm.tm_mday;
	uint32_t month_selected = tm.tm_mon + 1;
	uint32_t year_selected = tm.tm_year + 1900;
	uint32_t hour_selected = tm.tm_hour;
	uint32_t minute_selected = tm.tm_min;
	uint32_t seconds_selected = tm.tm_sec;
	
	uint32_t white = SDL_MapRGB(screen->format, 255,255,255);

	#define kSlash 10
	#define kColon 11
	#define kBar 12
	int blit(int i, int x, int y) {
		SDL_BlitSurface(digits, &(SDL_Rect){i*20,0,20,32}, screen, &(SDL_Rect){x,y});
		return x + 20;
	}
	void blitBar(int x, int y, int len) {
		int w = len * 20;
		SDL_FillRect(screen, &(SDL_Rect){x,y,w,5}, white);
		SDL_BlitSurface(digits, &(SDL_Rect){kBar*20,0,3,5}, screen, &(SDL_Rect){x,y});
		SDL_BlitSurface(digits, &(SDL_Rect){(kBar*20)+2,0,3,5}, screen, &(SDL_Rect){x+w-3,y});
	}
	int blitNumber(int num, int x, int y) {
		int n;
		if (num > 999) {
			n = num / 1000;
			num -= n * 1000;
			x = blit(n, x,y);
			
			n = num / 100;
			num -= n * 100;
			x = blit(n, x,y);
		}
		n = num / 10;
		num -= n * 10;
		x = blit(n, x,y);
		
		n = num;
		x = blit(n, x,y);
		
		return x;
	}
	void validate(void) {
		// leap year
		uint32_t february_days = 28;
		if ( ((year_selected % 4 == 0) && (year_selected % 100 != 0)) || (year_selected % 400 == 0)) february_days = 29;
	
		// day
		if (day_selected < 1) day_selected = 1;
		if (month_selected > 12) month_selected = 12;
		else if (month_selected < 1) month_selected = 1;
	
		if (year_selected > 2100) year_selected = 2100;
		else if (year_selected < 1970) year_selected = 1970;
	
		switch(month_selected)
		{
			case 2:
				if (day_selected > february_days) day_selected = february_days;
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if (day_selected > 30) day_selected = 30;
				break;
			default:
				if (day_selected > 31) day_selected = 31;
				break;
		}
	
		// time
		if (hour_selected > 23) hour_selected = 0;
		if (minute_selected > 59) minute_selected = 0;
		if (seconds_selected > 59) seconds_selected = 0;
	}
	
	SDL_EnableKeyRepeat(300,100);
	
	while(!quit) {
		unsigned long frame_start = SDL_GetTicks();
			
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_UP:
						switch(select_cursor) {
							case 0:
								year_selected++;
							break;
							case 1:
								month_selected++;
							break;
							case 2:
								day_selected++;
							break;
							case 3:
								hour_selected++;
							break;
							case 4:
								minute_selected++;
							break;
							case 5:
								seconds_selected++;
							break;
						}
						break;
						case SDLK_DOWN:
						switch(select_cursor) {
							case 0:
								year_selected--;
							break;
							case 1:
								month_selected--;
							break;
							case 2:
								day_selected--;
							break;
							case 3:
								hour_selected--;
							break;
							case 4:
								minute_selected--;
							break;
							case 5:
								seconds_selected--;
							break;
						}
						break;
						case SDLK_LEFT:
						select_cursor--;
						if (select_cursor < 0) select_cursor += 6;
						break;
						case SDLK_RIGHT:
						select_cursor++;
						if (select_cursor > 5) select_cursor -= 6;
						break;
						case SDLK_SPACE: // A
							save_changes = 1;
							quit = 1;
						break;
						case SDLK_LCTRL: // B
							quit = 1;
						break;
						default:
						break;
					}
				break;
				case SDL_QUIT:
					quit = 1;
				break;
			}
		}
		
		validate();
		
		// render
		SDL_BlitSurface(confirm, NULL, screen, NULL);
		
		// datetime
		int x = 130;
		int y = 185;
		
		x = blitNumber(year_selected, x,y);
		x = blit(kSlash, x,y);
		x = blitNumber(month_selected, x,y);
		x = blit(kSlash, x,y);
		x = blitNumber(day_selected, x,y);
		x += 20; // space
		x = blitNumber(hour_selected, x,y);
		x = blit(kColon, x,y);
		x = blitNumber(minute_selected, x,y);
		x = blit(kColon, x,y);
		x = blitNumber(seconds_selected, x,y);
		
		// cursor
		x = 130;
		y = 222;
		if (select_cursor>0) {
			x += 100; // YYYY/
			x += (select_cursor - 1) * 60;
		}
		blitBar(x,y, (select_cursor>0 ? 2 : 4));
		
		SDL_Flip(screen);

		// slow down to 60fps
		unsigned long frame_duration = SDL_GetTicks() - frame_start;
		#define kTargetFrameDuration 17
		if (frame_duration<kTargetFrameDuration) SDL_Delay(kTargetFrameDuration-frame_duration);
	}

	SDL_FreeSurface(confirm);
	SDL_FreeSurface(digits);
	SDL_Quit();
	
	if (save_changes) {
		char cmd[512];
		snprintf(cmd, sizeof(cmd), "date -s '%d-%d-%d %d:%d:%d';hwclock --utc -w", year_selected, month_selected, day_selected, hour_selected, minute_selected, seconds_selected);
		system(cmd);
	}
	
	return EXIT_SUCCESS;
}
