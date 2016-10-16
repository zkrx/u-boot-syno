// Copyright (c) 2007 Synology Inc. All rights reserved.

#ifdef SYNO_DISK_STATION

#include <common.h>
#include <syno_custom.h>

void syno_board_print(void)
{
    int tmp = 0;
    char szRev[16];
    int i = 0;
    int fanStatus[SYNO_MAX_FAN];

    printf("\n");

    printf("Synology Model: %s\n", model_name_get());

    for ( i = 0; i < SYNO_MAX_FAN; i++) {
	fanStatus[i] = -1;
    }
    tmp = syno_custom_fan_status_get(fanStatus, SYNO_MAX_FAN);
    if ( tmp > 0 ) {
	printf("Fan Status: Good\n");
    } else {
	printf("Fan Status: Not Good\n");
	for ( i = 0; i < SYNO_MAX_FAN; i++) {
		if ( fanStatus[i] == 1 ) {
			printf("Fan %d: Good\n", i+1);
		} else if ( fanStatus[i] == 0 ) {
			printf("Fan %d: Not Good\n", i+1);
		} 
	}
    }
}

void syno_bootargs_get(char *bootargs)
{
    syno_custom_bootargs_get(bootargs);
}

void syno_disk_led_set(int status)
{
    syno_custom_led_set(status);
}

void syno_bootargs_replace(char *bootargs, char *key, char *value)
{
	syno_custom_bootargs_replace(bootargs, key, value);
}

#endif
