#ifndef COMMON_H
#define COMMON_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

// platform independent
#define MIN_VOLUME 0
#define MAX_VOLUME 20
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 10

#define HINT_SLEEP "POWER"
#define HINT_RESUME "X"

#define SDLK_UNDEFINED -1

// must be unique except for absent buttons
#define MINUI_UP 		SDLK_UP
#define MINUI_DOWN 		SDLK_DOWN
#define MINUI_LEFT 		SDLK_LEFT
#define MINUI_RIGHT 	SDLK_RIGHT
#define MINUI_A 		SDLK_SPACE
#define MINUI_B 		SDLK_LCTRL
#define MINUI_X 		SDLK_LSHIFT
#define MINUI_Y 		SDLK_LALT
#define MINUI_START 	SDLK_RETURN
#define MINUI_SELECT 	SDLK_RCTRL
#define MINUI_L 		SDLK_e
#define MINUI_R 		SDLK_t
#define MINUI_MENU	 	SDLK_ESCAPE
#define MINUI_L2 		SDLK_TAB
#define MINUI_R2 		SDLK_BACKSPACE
#define MINUI_L3 		SDLK_UNDEFINED
#define MINUI_R3 		SDLK_UNDEFINED
#define MINUI_POWER 	SDLK_POWER

#define GOLD_TRIAD 0xd2,0xb4,0x6c
#define BRONZE_TRIAD 0x9f,0x89,0x52
#define WHITE_TRIAD 0xff,0xff,0xff
#define GRAY_TRIAD 0x66,0x66,0x66
#define SHADOW25_TRIAD 0xa1,0x87,0x47
#define SHADOW50_TRIAD 0x68,0x5a,0x35

typedef enum ButtonIndex {
	kButtonUp = 0,
	kButtonDown,
	kButtonLeft,
	kButtonRight,
	kButtonA,
	kButtonB,
	kButtonX,
	kButtonY,
	kButtonStart,
	kButtonSelect,
	kButtonL,
	kButtonR,
	kButtonMenu,
	kButtonL2,
	kButtonR2,
	kButtonL3,
	kButtonR3,
	kButtonPower,
	kButtonCount,
} ButtonIndex;

// map actions to platform specific buttons here
// TODO: considering moving resume to START on all platforms?
// but then we need to extend the can_start and can_select behavior too...
#define kButtonSleep kButtonPower
#define kButtonResume kButtonX
#define kButtonAltEmu kButtonY

typedef struct UnionPaths {
	char rootDir[256];
	char userdataDir[256];
	char resDir[256];
	char romsDir[256];
	char paksDir[256];
	char recentPath[256];
	char fauxRecentDir[256];
} UnionPaths;

// TODO: pull paths out of Paths struct, remove unnecessary sprintf calls
#define kLastPath "/tmp/last.txt" // transient
#define kChangeDiscPath "/tmp/change_disc.txt"
#define kResumeSlotPath "/tmp/mmenu_slot.txt"
#define kAutoResumePath "/mnt/SDCARD/.userdata/.miniui/auto_resume.txt"
#define kAutoResumeSlot "9"

// NOTE: this was a huge, dumb refactor
typedef struct UnionScreen {
	int width;
	int height;
	
	struct {
		
		// TODO: break up into {size,oy} structs by size
		int small_size;
		int medium_size;
		int large_size;
		int small_oy;
		int medium_oy;
		int large_oy;
		
		struct {
			int ox;
			int oy;
		} shadow;
	} font;
	
	struct {
		int size;
		struct {
			int oy;
			int ox_A;
			int ox_B;
			int ox_X;
			int ox_Y;
			int ox_R;
		} text;
	} button;
	
	struct {
		int pad_width;
		int text_oy;
	} pill;
	
	struct {
		int ox;
		int text_oy;
	} hint;
	
	struct {
		int width;
		int height;
		struct {
			int ox;
			int oy;
		} icon;
		struct {
			int ox;
			int oy;
		} bar;
	} settings;
	
	struct {
		int top; // TODO: why is this top and not y?
		int left;
		int right;
		int gutter;
	} buttons;
	
	struct { // main
		struct {
			int x;
			int y;
		} settings;

		struct {
			int x;
			int y;
		} battery;
		
		struct {
			int top_y;
			int bottom_y;
		} rule;

		struct {
			int x;
			int y;
		} logo;

		struct {
			int row_count;
			int y;
			int ox;
			int oy;
			int row_height;
			int alt_ox;
			int alt_oy;
		} list;
	} main;
	
	struct { // menu
		struct {
			int x;
			int y;
		} settings;

		struct {
			int x;
			int y;
		} battery;
		
		struct {
			int top_y;
			int bottom_y;
		} rule;
		
		struct {
			int top; // TODO: why is this top and not y?
		} buttons;
		
		struct {
			int ox;
			int oy;
		} disc;
		
		struct {
			int x;
			int y;
			int width;
			int height;
			int inset;
		} preview;
	
		struct {
			int x;
			int y;
			int ox;
		} slots;
		
		struct {
			int x;
			int y;
			int width;
		} title;
		
		struct {
			int x;
			int y;
			int width;
			int height;
		} window;
		
		struct {
			int x;
			int y;
			int oy;
			int line_height;
			int row_height;
		} list;
		
		struct {
			int ox; // NOTE: from right edge
			int oy;
		} arrow;
		
		int bar_height;
	} menu;
		
	struct {
		int line_height;
	} body;

} UnionScreen;

extern UnionPaths Paths;
extern UnionScreen Screen;

void Input_reset(void);
void Input_poll(void);
int Input_anyPressed(void);
int Input_justPressed(ButtonIndex btn);
int Input_justRepeated(ButtonIndex btn);
int Input_isPressed(ButtonIndex btn);
int Input_justReleased(ButtonIndex btn);

int prefixMatch(char* pre, char* str);
int suffixMatch(char* suf, char* str);
int exactMatch(char* str1, char* str2);
int hide(char* file_name);

void getDisplayName(const char* in_name, char* out_name);
void getEmuName(const char* in_name, char* out_name);

void normalizeNewline(char* line);
void trimTrailingNewlines(char* line);

int exists(char* path);
void putFile(char* path, char* contents);
void getFile(char* path, char* buffer, size_t buffer_size);
void putInt(char* path, int value);
int getInt(char* path);

void GFX_init(void);
void GFX_ready(void);
void GFX_quit(void);
void GFX_clear(void);
SDL_Surface* GFX_loadImage(char* path); // only meant for system images, not arbitrary images like save state previews!
void GFX_blitRule(SDL_Surface* surface, int y);
int GFX_blitHint(SDL_Surface* surface, char* htxt, int x, int y);
int GFX_blitPill(SDL_Surface* surface, char* btxt, char* htxt, int x, int y);
int GFX_blitButton(SDL_Surface* surface, char* btxt, char* htxt, int x, int y, int bx);
void GFX_blitMenu(SDL_Surface* surface, char* name, char* path, int conflict, int row, int selected_row, int has_alt, int use_alt);
void GFX_blitWindow(SDL_Surface* surface, int x, int y, int width, int height, int color); // color=0:black,1:white
void GFX_blitBodyCopy(SDL_Surface* surface, char* text, int x, int y, int width, int height); // gold, centered in rect, multiline okay!
int GFX_blitText(SDL_Surface* surface, char* text, int size, int x, int y, int width, int color, int has_shadow); // size=0:small,1:medium,2:large width=0:left,>0:center,<0:right, color=0:white,1:gold, no multiline

SDL_Surface* GFX_getText(char* text, int size, int color);
void GFX_blitBattery(SDL_Surface* surface, int x, int y);
void GFX_blitSettings(SDL_Surface* surface, int x, int y, int icon, int value, int min_value, int max_value); // icon=0:brightness,1:volume,2:mute

void waitForWake(void);
void fauxSleep(void);
int getSmoothBatteryLevel(void);

int isCharging(void);
int getBatteryLevel(void);

void powerOff(void);

void enterSleep(void);
void exitSleep(void);


int preventAutosleep(void);

#endif