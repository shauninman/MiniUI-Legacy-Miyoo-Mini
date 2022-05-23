#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <msettings.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/common.h"

#include "mmenu.h"

static SDL_Surface* overlay;
static SDL_Surface* screen;
static SDL_Surface* slot_overlay;
static SDL_Surface* slot_dots;
static SDL_Surface* slot_dot_selected;
static SDL_Surface* arrow;
static SDL_Surface* arrow_highlighted;
static SDL_Surface* no_preview;
static SDL_Surface* empty_slot;

enum {
	kItemContinue,
	kItemSave,
	kItemLoad,
	kItemAdvanced,
	kItemExitGame,
};
#define kItemCount 5
static char* items[kItemCount];

#define kSlotCount 8
static int slot = 0;
static int is_simple = 0;

__attribute__((constructor)) static void init(void) {
	void* librt = dlopen("librt.so.1", RTLD_LAZY | RTLD_GLOBAL); // shm
	void* libmsettings = dlopen("libmsettings.so", RTLD_LAZY | RTLD_GLOBAL);
	InitSettings();
	
	is_simple = exists(kEnableSimpleModePath);
	
	items[kItemContinue] 	= "Continue";
	items[kItemSave] 		= "Save";
	items[kItemLoad] 		= "Load";
	items[kItemAdvanced] 	= is_simple ? "Reset" : "Advanced";
	items[kItemExitGame] 	= "Quit";
	
	GFX_init();
	
	overlay = SDL_CreateRGBSurface(SDL_SWSURFACE, Screen.width, Screen.height, 16, 0, 0, 0, 0);
	SDL_SetAlpha(overlay, SDL_SRCALPHA, 0x80);
	SDL_FillRect(overlay, NULL, 0);
	
	slot_overlay = GFX_loadImage("slot-overlay.png");
	slot_dots = GFX_loadImage("slot-dots.png");
	slot_dot_selected = GFX_loadImage("slot-dot-selected.png");
	arrow = GFX_loadImage("arrow.png");
	arrow_highlighted = GFX_loadImage("arrow-highlighted.png");
	no_preview = GFX_getText("No Preview", 0, 1);
	empty_slot = GFX_getText("Empty Slot", 0, 1);
}
__attribute__((destructor)) static void quit(void) {
	SDL_FreeSurface(overlay);
	SDL_FreeSurface(slot_overlay);
	SDL_FreeSurface(slot_dots);
	SDL_FreeSurface(slot_dot_selected);
	SDL_FreeSurface(arrow);
	SDL_FreeSurface(arrow_highlighted);
	SDL_FreeSurface(no_preview);
	SDL_FreeSurface(empty_slot);
}

typedef struct __attribute__((__packed__)) uint24_t {
	uint8_t a,b,c;
} uint24_t;
static SDL_Surface* createThumbnail(SDL_Surface* src_img) {
	SDL_Surface* dst_img = SDL_CreateRGBSurface(0,Screen.width/2, Screen.height/2,src_img->format->BitsPerPixel,src_img->format->Rmask,src_img->format->Gmask,src_img->format->Bmask,src_img->format->Amask);

	uint8_t* src_px = src_img->pixels;
	uint8_t* dst_px = dst_img->pixels;
	int step = dst_img->format->BytesPerPixel;
	int step2 = step * 2;
	int stride = src_img->pitch;
	for (int y=0; y<dst_img->h; y++) {
		for (int x=0; x<dst_img->w; x++) {
			switch(step) {
				case 1:
					*dst_px = *src_px;
					break;
				case 2:
					*(uint16_t*)dst_px = *(uint16_t*)src_px;
					break;
				case 3:
					*(uint24_t*)dst_px = *(uint24_t*)src_px;
					break;
				case 4:
					*(uint32_t*)dst_px = *(uint32_t*)src_px;
					break;
			}
			dst_px += step;
			src_px += step2;
		}
		src_px += stride;
	}

	return dst_img;
}

static int state_support = 1;
static int disable_poweroff = 0;
void ShowWarning(void) {
	state_support = 0;
	disable_poweroff = 1;
	disablePoweroff();
	
	screen = SDL_GetVideoSurface();
	GFX_ready();
	
	SDL_Surface* okay = GFX_loadImage("okay.png");
	SDL_BlitSurface(okay, NULL, screen, NULL);
	GFX_blitBodyCopy(screen, "This pak does not support\nsave states, auto-power off\nor quicksave and resume.", 0,0,screen->w,406);
	SDL_Flip(screen);

	SDL_Event event;
	int warned = 0;
	while (!warned) {
		while (SDL_PollEvent(&event)) {
			if (event.type==SDL_KEYDOWN) {
				SDLKey key = event.key.keysym.sym;
				if (key==MINUI_A) {
					warned = 1;
					break;
				}
			}
		}
		SDL_Delay(200);
	}
	SDL_FreeSurface(okay);
	
	// TODO: show warning
	// TODO: set a flag that saves aren't available
	// TODO: disable auto-power off
}

MenuReturnStatus ShowMenu(char* rom_path, char* save_path_template, SDL_Surface* optional_snapshot, MenuRequestState requested_state, AutoSave_t autosave) {
	screen = SDL_GetVideoSurface();
	
	GFX_ready();
	
	SDL_EnableKeyRepeat(300,100);
	
	// path and string things
	char* tmp;
	char rom_file[256]; // with extension
	char rom_name[256]; // without extension or cruft
	char slot_path[256];
	char emu_name[256];
	char mmenu_dir[256];
	
	// filename
	tmp = strrchr(rom_path,'/');
	if (tmp==NULL) tmp = rom_path;
	else tmp += 1;
	strcpy(rom_file, tmp);
	
	getEmuName(rom_path, emu_name);
	sprintf(mmenu_dir, "%s/.mmenu/%s", getenv("USERDATA_PATH"), emu_name);
	mkdir(mmenu_dir, 0755);

	// does this game have an m3u?
	int rom_disc = -1;
	int disc = rom_disc;
	int total_discs = 0;
	char disc_name[16];
	char* disc_paths[9]; // up to 9 paths, Arc the Lad Collection is 7 discs

	// construct m3u path based on parent directory
	// essentially hasM3u() from MiniUI but we use the building blocks as well
	char m3u_path[256];
	strcpy(m3u_path, rom_path);
	tmp = strrchr(m3u_path, '/') + 1;
	tmp[0] = '\0';

	// path to parent directory
	char base_path[256]; // used below too when status==kItemSave
	strcpy(base_path, m3u_path);

	tmp = strrchr(m3u_path, '/');
	tmp[0] = '\0';

	// get parent directory name
	char dir_name[256];
	tmp = strrchr(m3u_path, '/');
	strcpy(dir_name, tmp);

	// dir_name is also our m3u file name
	tmp = m3u_path + strlen(m3u_path); 
	strcpy(tmp, dir_name);

	// add extension
	tmp = m3u_path + strlen(m3u_path);
	strcpy(tmp, ".m3u");
	
	if (exists(m3u_path)) {
		// share saves across multi-disc games
		strcpy(rom_file, dir_name);
		tmp = rom_file + strlen(rom_file);
		strcpy(tmp, ".m3u");
		
		//read m3u file
		FILE* file = fopen(m3u_path, "r");
		if (file) {
			char line[256];
			while (fgets(line,256,file)!=NULL) {
				int len = strlen(line);
				if (len>0 && line[len-1]=='\n') {
					line[len-1] = 0; // trim newline
					len -= 1;
					if (len>0 && line[len-1]=='\r') {
						line[len-1] = 0; // trim Windows newline
						len -= 1;
					}
				}
				if (len==0) continue; // skip empty lines
		
				char disc_path[256];
				strcpy(disc_path, base_path);
				tmp = disc_path + strlen(disc_path);
				strcpy(tmp, line);
				
				// found a valid disc path
				if (exists(disc_path)) {
					disc_paths[total_discs] = strdup(disc_path);
					// matched our current disc
					if (exactMatch(disc_path, rom_path)) {
						rom_disc = total_discs;
						disc = rom_disc;
						sprintf(disc_name, "Disc %i", disc+1);
					}
					total_discs += 1;
				}
			}
			fclose(file);
		}
	}
	
	// m3u path may change rom_file
	sprintf(slot_path, "%s/%s.txt", mmenu_dir, rom_file);
	getDisplayName(rom_file, rom_name);
	
	// cache static elements
	
	// NOTE: original screen copying logic
	// SDL_Surface* copy = SDL_CreateRGBSurface(SDL_SWSURFACE, Screen.width, Screen.height, 16, 0, 0, 0, 0);
	// SDL_BlitSurface(screen, NULL, copy, NULL);

	// NOTE: copying the screen to a new surface caused a 15fps drop in PocketSNES
	// on the 280M running stock OpenDingux after opening the menu, 
	// tried ConvertSurface and DisplaySurface too
	// only this direct copy worked without tanking the framerate
	int copy_bytes = screen->h * screen->pitch;
	void* copy_pixels = malloc(copy_bytes);
	memcpy(copy_pixels, screen->pixels, copy_bytes);
	SDL_Surface* copy = SDL_CreateRGBSurfaceFrom(copy_pixels, screen->w,screen->h, screen->format->BitsPerPixel, screen->pitch, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	
	SDL_Surface* cache = SDL_CreateRGBSurface(SDL_SWSURFACE, Screen.width, Screen.height, 16, 0, 0, 0, 0);
	SDL_BlitSurface(copy, NULL, cache, NULL);
	SDL_BlitSurface(overlay, NULL, cache, NULL);

	SDL_FillRect(cache, &(SDL_Rect){0,0,Screen.width,Screen.menu.bar_height}, 0);
	GFX_blitRule(cache, Screen.menu.rule.top_y);

	GFX_blitText(cache, rom_name, 1, Screen.menu.title.x, Screen.menu.title.y, Screen.menu.title.width, 1, 0);
	
	GFX_blitWindow(cache, Screen.menu.window.x, Screen.menu.window.y, Screen.menu.window.width, Screen.menu.window.height, 1);
	
	SDL_FillRect(cache, &(SDL_Rect){0,Screen.height-Screen.menu.bar_height,Screen.width,Screen.menu.bar_height}, 0);
	GFX_blitRule(cache, Screen.menu.rule.bottom_y);
	
	Input_reset();
	
	int gold_rgb = SDL_MapRGB(screen->format, GOLD_TRIAD);
	int gray_rgb = SDL_MapRGB(screen->format, DISABLED_TRIAD);
	SDL_Color gold = (SDL_Color){GOLD_TRIAD};
	SDL_Color white = (SDL_Color){WHITE_TRIAD};
	
	int status = kStatusContinue;
	int selected = 0; // resets every launch
	if (exists(slot_path)) slot = getInt(slot_path);
	if (slot==8) slot = 0;
	
	// inline functions? okay.
	void SystemRequest(MenuRequestState request) {
		autosave();
		putFile(kAutoResumePath, rom_path + strlen(Paths.rootDir));
		if (request==kRequestSleep) {
			fauxSleep();
			unlink(kAutoResumePath);
		}
		else powerOff();
	}
	
	if (requested_state!=kRequestMenu) { // sleep or poweroff
		if (disable_poweroff && requested_state==kRequestPowerOff) return kStatusContinue;
		SystemRequest(requested_state);
		if (requested_state==kRequestPowerOff) return kStatusPowerOff;
	}
	
	char save_path[256];
	char bmp_path[324];
	char txt_path[256];
	int save_exists = 0;
	int preview_exists = 0;
	
	// TODO: remove is_locked logic
	
	int quit = 0;
	int dirty = 1;
	int show_setting = 0; // 1=brightness,2=volume
	int setting_value = 0;
	int setting_min = 0;
	int setting_max = 0;
	int select_is_locked = 0; // rs90-only
	unsigned long cancel_start = SDL_GetTicks();
	int was_charging = isCharging();
	unsigned long charge_start = SDL_GetTicks();
	unsigned long power_start = 0;
	while (!quit) {
		unsigned long frame_start = SDL_GetTicks();
		int select_was_pressed = Input_isPressed(kButtonSelect); // rs90-only
		
		Input_poll();
		
		// rs90-only
		if (select_was_pressed && Input_justReleased(kButtonSelect) && Input_justPressed(kButtonL)) {
			select_is_locked = 1;
		}
		else if (select_is_locked && Input_justReleased(kButtonL)) {
			select_is_locked = 0;
		}
		
		if (Input_justPressed(kButtonUp)) {
			selected -= 1;
			if (selected<0) selected += kItemCount;
			dirty = 1;
		}
		else if (Input_justPressed(kButtonDown)) {
			selected += 1;
			if (selected>=kItemCount) selected -= kItemCount;
			dirty = 1;
		}
		else if (Input_justPressed(kButtonLeft)) {
			if (total_discs>1 && selected==kItemContinue) {
				disc -= 1;
				if (disc<0) disc += total_discs;
				dirty = 1;
				sprintf(disc_name, "Disc %i", disc+1);
			}
			else if (selected==kItemSave || selected==kItemLoad) {
				slot -= 1;
				if (slot<0) slot += kSlotCount;
				dirty = 1;
			}
		}
		else if (Input_justPressed(kButtonRight)) {
			if (total_discs>1 && selected==kItemContinue) {
				disc += 1;
				if (disc==total_discs) disc -= total_discs;
				dirty = 1;
				sprintf(disc_name, "Disc %i", disc+1);
			}
			else if (selected==kItemSave || selected==kItemLoad) {
				slot += 1;
				if (slot>=kSlotCount) slot -= kSlotCount;
				dirty = 1;
			}
		}
		
		if (dirty && state_support && (selected==kItemSave || selected==kItemLoad)) {
			sprintf(save_path, save_path_template, slot);
			sprintf(bmp_path, "%s/%s.%d.bmp", mmenu_dir, rom_file, slot);
			sprintf(txt_path, "%s/%s.%d.txt", mmenu_dir, rom_file, slot);
		
			save_exists = exists(save_path);
			preview_exists = save_exists && exists(bmp_path);
			// printf("save_path: %s (%i)\n", save_path, save_exists);
			// printf("bmp_path: %s (%i)\n", bmp_path, preview_exists);
		}
		
		if (Input_justPressed(kButtonB) || Input_justPressed(kButtonMenu)) {
			status = kStatusContinue;
			quit = 1;
		}
		else if (Input_justPressed(kButtonA)) {
			switch(selected) {
				case kItemContinue:
				if (total_discs && rom_disc!=disc) {
						status = kStatusChangeDisc;
						char* disc_path = disc_paths[disc];
						putFile(kChangeDiscPath, disc_path);
					}
					else {
						status = kStatusContinue;
					}
					quit = 1;
				break;
				case kItemSave:
				if (state_support) {
					status = kStatusSaveSlot + slot;
					SDL_Surface* preview = createThumbnail(optional_snapshot ? optional_snapshot : copy);
					SDL_RWops* out = SDL_RWFromFile(bmp_path, "wb");
					if (total_discs) {
						char* disc_path = disc_paths[disc];
						putFile(txt_path, disc_path + strlen(base_path));
						sprintf(bmp_path, "%s/%s.%d.bmp", mmenu_dir, rom_file, slot);
					}
					SDL_SaveBMP_RW(preview, out, 1);
					SDL_FreeSurface(preview);
					putInt(slot_path, slot);
					quit = 1;
				}
				break;
				case kItemLoad:
				if (state_support) {
					if (save_exists && total_discs) {
						char slot_disc_name[256];
						getFile(txt_path, slot_disc_name, 256);
						char slot_disc_path[256];
						if (slot_disc_name[0]=='/') strcpy(slot_disc_path, slot_disc_name);
						else sprintf(slot_disc_path, "%s%s", base_path, slot_disc_name);
						char* disc_path = disc_paths[disc];
						if (!exactMatch(slot_disc_path, disc_path)) {
							putFile(kChangeDiscPath, slot_disc_path);
						}
					}
					status = kStatusLoadSlot + slot;
					putInt(slot_path, slot);
					quit = 1;
				}
				break;
				case kItemAdvanced:
					status = is_simple ? kStatusResetGame : kStatusOpenMenu;
					quit = 1;
				break;
				case kItemExitGame:
					status = kStatusExitGame;
					quit = 1;
				break;
			}
			if (quit) break;
		}
		
		unsigned long now = SDL_GetTicks();
		if (Input_anyPressed()) cancel_start = now;

		#define kChargeDelay 1000
		if (dirty || now-charge_start>=kChargeDelay) {
			int is_charging = isCharging();
			if (was_charging!=is_charging) {
				was_charging = is_charging;
				dirty = 1;
			}
			charge_start = now;
		}

		if (!disable_poweroff && power_start && now-power_start>=1000) {
			SystemRequest(kRequestPowerOff);
			status = kStatusPowerOff;
			quit = 1;
		}
		if (Input_justPressed(kButtonSleep)) {
			power_start = now;
		}
		
		#define kSleepDelay 30000
		if (now-cancel_start>=kSleepDelay && preventAutosleep()) cancel_start = now;
		
		if (now-cancel_start>=kSleepDelay || Input_justReleased(kButtonSleep)) // || Input_justPressed(kButtonMenu)) 
		{
			SystemRequest(kRequestSleep);
			cancel_start = SDL_GetTicks();
			power_start = 0;
			dirty = 1;
		}
		
		int old_setting = show_setting;
		int old_value = setting_value;
		show_setting = 0;
		if (Input_isPressed(kButtonStart) && Input_isPressed(kButtonSelect)) {
			// buh
		}
		else if (Input_isPressed(kButtonStart)) {
			show_setting = 1;
			setting_value = GetBrightness();
			setting_min = MIN_BRIGHTNESS;
			setting_max = MAX_BRIGHTNESS;
		}
		else if (Input_isPressed(kButtonSelect) || select_is_locked) {
			show_setting = 2;
			setting_value = GetVolume();
			setting_min = MIN_VOLUME;
			setting_max = MAX_VOLUME;
		}
		if (old_setting!=show_setting || old_value!=setting_value) dirty = 1;
		
		if (dirty) {
			dirty = 0;
			SDL_BlitSurface(cache, NULL, screen, NULL);
			GFX_blitBattery(screen, Screen.menu.battery.x, Screen.menu.battery.y);
			
			if (show_setting) {
				GFX_blitSettings(screen, Screen.menu.settings.x, Screen.menu.settings.y, show_setting==1?0:(setting_value>0?1:2), setting_value,setting_min,setting_max);
			}
			
			// list
			SDL_Surface* text;
			for (int i=0; i<kItemCount; i++) {
				char* item = items[i];
				int disabled = !state_support && (i==kItemSave || i==kItemLoad);
				int color = disabled ? -1 : 1; // gray or gold
				if (i==selected) {
					int bg_color_rgb = disabled ? gray_rgb : gold_rgb;
					SDL_FillRect(screen, &(SDL_Rect){Screen.menu.window.x,Screen.menu.list.y+(i*Screen.menu.list.line_height)-((Screen.menu.list.row_height-Screen.menu.list.line_height)/2),Screen.menu.window.width,Screen.menu.list.row_height}, bg_color_rgb);
					if (!disabled) color = 0; // white
				}
				
				GFX_blitText(screen, item, 2, Screen.menu.list.x, Screen.menu.list.y+(i*Screen.menu.list.line_height)+Screen.menu.list.oy, 0, color, i==selected);
				
				if ((state_support && (i==kItemSave || i==kItemLoad)) || (total_discs>1 && i==kItemContinue)) {
					SDL_BlitSurface(i==selected?arrow_highlighted:arrow, NULL, screen, &(SDL_Rect){Screen.menu.window.x+Screen.menu.window.width-(arrow->w+Screen.menu.arrow.ox),Screen.menu.list.y+(i*Screen.menu.list.line_height)+Screen.menu.arrow.oy});
				}
			}
			
			// disc change
			if (total_discs>1 && selected==kItemContinue) {
				GFX_blitWindow(screen, Screen.menu.preview.x, Screen.menu.preview.y, Screen.menu.preview.width, Screen.menu.list.row_height+(Screen.menu.disc.oy*2), 1);
				GFX_blitText(screen, disc_name, 2, Screen.menu.preview.x+Screen.menu.disc.ox, Screen.menu.list.y+Screen.menu.list.oy, 0, 1, 0);
			}
			// slot preview
			else if (state_support && (selected==kItemSave || selected==kItemLoad)) {
				// preview window
				SDL_Rect preview_rect = {Screen.menu.preview.x+Screen.menu.preview.inset,Screen.menu.preview.y+Screen.menu.preview.inset};
				GFX_blitWindow(screen, Screen.menu.preview.x, Screen.menu.preview.y, Screen.menu.preview.width, Screen.menu.preview.height, 1);
				
				if (preview_exists) { // has save, has preview
					SDL_Surface* preview = IMG_Load(bmp_path);
					if (!preview) printf("IMG_Load: %s\n", IMG_GetError());
					SDL_BlitSurface(preview, NULL, screen, &preview_rect);
					SDL_FreeSurface(preview);
				}
				else {
					int hw = Screen.width / 2;
					int hh = Screen.height / 2;
					SDL_FillRect(screen, &(SDL_Rect){Screen.menu.preview.x+Screen.menu.preview.inset,Screen.menu.preview.y+Screen.menu.preview.inset,hw,hh}, 0);
					if (save_exists) { // has save but no preview
						SDL_BlitSurface(no_preview, NULL, screen, &(SDL_Rect){
							Screen.menu.preview.x+Screen.menu.preview.inset+(hw-no_preview->w)/2,
							Screen.menu.preview.y+Screen.menu.preview.inset+(hh-no_preview->h)/2
						});
					}
					else { // no save
						SDL_BlitSurface(empty_slot, NULL, screen, &(SDL_Rect){
							Screen.menu.preview.x+Screen.menu.preview.inset+(hw-empty_slot->w)/2,
							Screen.menu.preview.y+Screen.menu.preview.inset+(hh-empty_slot->h)/2
						});
					}
				}
				
				SDL_BlitSurface(slot_overlay, NULL, screen, &preview_rect);
				SDL_BlitSurface(slot_dots, NULL, screen, &(SDL_Rect){Screen.menu.slots.x,Screen.menu.slots.y});
				SDL_BlitSurface(slot_dot_selected, NULL, screen, &(SDL_Rect){Screen.menu.slots.x+(Screen.menu.slots.ox*slot),Screen.menu.slots.y});
			}
			
			GFX_blitPill(screen, HINT_SLEEP, "SLEEP", Screen.buttons.left, Screen.menu.buttons.top);
			// TODO: change ACT to OKAY?
			int button_width = GFX_blitButton(screen, "A", "ACT", -Screen.buttons.right, Screen.menu.buttons.top, Screen.button.text.ox_A);
			GFX_blitButton(screen, "B", "BACK", -(Screen.buttons.right+button_width+Screen.buttons.gutter),Screen.menu.buttons.top, Screen.button.text.ox_B);
			// TODO: /can this be cached?
			
			SDL_Flip(screen);
		}
		
		// slow down to 60fps
		unsigned long frame_duration = SDL_GetTicks() - frame_start;
		#define kTargetFrameDuration 17
		if (frame_duration<kTargetFrameDuration) SDL_Delay(kTargetFrameDuration-frame_duration);
	}
	
	// redraw original screen before returning
	if (status!=kStatusPowerOff) {
		SDL_FillRect(screen, NULL, 0);
		SDL_BlitSurface(copy, NULL, screen, NULL);
		SDL_Flip(screen);
	}

	SDL_FreeSurface(cache);
	// NOTE: copy->pixels was manually malloc'd so it must be manually freed too
	SDL_FreeSurface(copy);
	free(copy_pixels); 
	
	SDL_EnableKeyRepeat(0,0);
	
	// if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen); // fix for regba?
	
	return status;
}

int ResumeSlot(void) {
	if (!exists(kResumeSlotPath)) return -1;
	
	slot = getInt(kResumeSlotPath);
	unlink(kResumeSlotPath);

	return slot;
}

int ChangeDisc(char* disc_path) {
	if (!exists(kChangeDiscPath)) return 0;
	getFile(kChangeDiscPath, disc_path, 256);
	return 1;
}