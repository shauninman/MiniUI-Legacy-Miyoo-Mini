// miyoomini/keymon.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>

#include <msettings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pthread.h>

//	Button Defines
#define	BUTTON_MENU		KEY_ESC
#define	BUTTON_POWER	KEY_POWER
#define	BUTTON_SELECT	KEY_RIGHTCTRL
#define	BUTTON_START	KEY_ENTER
#define	BUTTON_L1		KEY_E
#define	BUTTON_R1		KEY_T
#define	BUTTON_L2		KEY_TAB
#define	BUTTON_R2		KEY_BACKSPACE

//	for keyshm
#define VOLUME		0
#define BRIGHTNESS	1
#define VOLMAX		20
#define BRIMAX		10

//	for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

//	for button_flag
#define SELECT_BIT	0
#define START_BIT	1
#define SELECT		(1<<SELECT_BIT)
#define START		(1<<START_BIT)

//	for DEBUG
//#define	DEBUG
#ifdef	DEBUG
#define ERROR(str)	fprintf(stderr,str"\n"); quit(EXIT_FAILURE)
#else
#define ERROR(str)	quit(EXIT_FAILURE)
#endif

//	Global Variables
typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)

static SAR_ADC_CONFIG_READ  adcCfg = {0,0};
static int sar_fd = 0;
static struct input_event	ev;
static int	input_fd = 0;
static pthread_t adc_pt;

void quit(int exitcode) {
	pthread_cancel(adc_pt);
	pthread_join(adc_pt, NULL);
	QuitSettings();
	
	if (input_fd > 0) close(input_fd);
	if (sar_fd > 0) close(sar_fd);
	exit(exitcode);
}

static void initADC(void) {
	sar_fd = open("/dev/sar", O_WRONLY);
	ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
}
static void checkADC(void) {
	ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);
	
	int adc_fd = open("/tmp/adc", O_CREAT | O_WRONLY);
	if (adc_fd>0) {
		char val[8];
		sprintf(val, "%d", adcCfg.adc_value);
		write(adc_fd, val, strlen(val));
		close(adc_fd);
	}
}
static void* runADC(void *arg) {
	while(1) {
		sleep(5);
		checkADC();
	}
	return 0;
}

int main (int argc, char *argv[]) {
	initADC();
	checkADC();
	pthread_create(&adc_pt, NULL, &runADC, NULL);
	
	// Set Initial Volume / Brightness
	InitSettings();
	
	input_fd = open("/dev/input/event0", O_RDONLY);

	// Main Loop
	register uint32_t val;
	register uint32_t button_flag = 0;
	register uint32_t menu_pressed = 0;
	register uint32_t power_pressed = 0;
	uint32_t repeat_LR = 0;
	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		val = ev.value;
		if (( ev.type != EV_KEY ) || ( val > REPEAT )) continue;
		switch (ev.code) {
		case BUTTON_MENU:
			if ( val != REPEAT ) menu_pressed = val;
			break;
		case BUTTON_POWER:
			if ( val != REPEAT ) power_pressed = val;
			break;
		case BUTTON_SELECT:
			if ( val != REPEAT ) {
				button_flag = button_flag & (~SELECT) | (val<<SELECT_BIT);
			}
			break;
		case BUTTON_START:
			if ( val != REPEAT ) {
				button_flag = button_flag & (~START) | (val<<START_BIT);
			} 
			break;
		case BUTTON_L1:
		case BUTTON_L2:
			if ( val == REPEAT ) {
				// Adjust repeat speed to 1/2
				val = repeat_LR;
				repeat_LR ^= PRESSED;
			} else {
				repeat_LR = 0;
			}
			if ( val == PRESSED ) {
				switch (button_flag) {
				case SELECT:
					// SELECT + L : volume down
					val = GetVolume();
					if (val>0) SetVolume(--val);
					break;
				case START:
					// START + L : brightness down
					val = GetBrightness();
					if (val>0) SetBrightness(--val);
					break;
				default:
					break;
				}
			}
			break;
		case BUTTON_R1:
		case BUTTON_R2:
			if ( val == REPEAT ) {
				// Adjust repeat speed to 1/2
				val = repeat_LR;
				repeat_LR ^= PRESSED;
			} else {
				repeat_LR = 0;
			}
			if ( val == PRESSED ) {
				switch (button_flag) {
				case SELECT:
					// SELECT + R : volume up
					val = GetVolume();
					if (val<VOLMAX) SetVolume(++val);
					break;
				case START:
					// START + R : brightness up
					val = GetBrightness();
					if (val<BRIMAX) SetBrightness(++val);
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
		
		if (menu_pressed && power_pressed) {
			menu_pressed = power_pressed = 0;
			system("sync && reboot");
			while (1) pause();
		}
	}
	ERROR("Failed to read input event");
}
