#define DISK_LED_OFF               0
#define DISK_LED_GREEN_SOLID       1
#define DISK_LED_ORANGE_SOLID      2

extern int SYNO_CTRL_FAN_STATUS_GET(int index, int *pValue);
extern int SYNO_CTRL_INTERNAL_HDD_LED_SET(int index, int status);
extern int SYNO_CTRL_EXT_CHIP_HDD_LED_SET(int index, int status);

void syno_board_print(void);
void syno_bootargs_get(char *bootargs);
void syno_disk_led_set(int status);
void syno_bootargs_replace(char *bootargs, char *key, char *value);

extern void syno_custom_bootargs_get(char *bootargs);
extern void syno_custom_led_set(int status);
extern int syno_custom_fan_status_get(int *pFanStatus, int size);

extern char* model_name_get(void);
extern int fan_status_get(int fanno);
extern char* get_random_ethaddr(int);
