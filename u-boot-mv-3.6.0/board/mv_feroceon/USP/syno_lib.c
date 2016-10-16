// Copyright (c) 2007 Synology Inc. All rights reserved.

#ifdef SYNO_DISK_STATION

#include <common.h>
#include <syno_custom.h>

char* model_name_get(void)
{
#if defined(SYNO_HW_VERSION_109)
	return "DS109";
#elif defined(SYNO_HW_VERSION_209)
	return "DS209";
#elif defined(SYNO_HW_VERSION_409slim)
	return "DS409slim";
#elif defined(SYNO_HW_VERSION_409)
	return "DS409";
#elif defined(SYNO_HW_VERSION_RS409)
	return "RS409";
#elif defined(SYNO_HW_VERSION_RS409r1)
	return "RS409r1";
#elif defined(SYNO_HW_VERSION_509)
	return "DS509";
#elif defined(SYNO_HW_VERSION_110j)
	return "DS110j";
#elif defined(SYNO_HW_VERSION_210j)
	return "DS210j";
#elif defined(SYNO_HW_VERSION_110jr1)
	return "DS110jr1";
#elif defined(SYNO_HW_VERSION_110jr2)
	return "DS110jr2";
#elif defined(SYNO_HW_VERSION_210jr1)
	return "DS210jr1";
#elif defined(SYNO_HW_VERSION_210jr2)
	return "DS210jr2";
#elif defined(SYNO_HW_VERSION_410j)
	return "DS410j";
#elif defined(SYNO_HW_VERSION_110)
	return "DS110";
#elif defined(SYNO_HW_VERSION_211)
	return "DS211";
#elif defined(SYNO_HW_VERSION_111)
	return "DS111";
#elif defined(SYNO_HW_VERSION_411slim)
	return "DS411slim";
#elif defined(SYNO_HW_VERSION_411)
	return "DS411";
#elif defined(SYNO_HW_VERSION_211p)
	return "DS211+";
#elif defined(SYNO_HW_VERSION_211pr1)
	return "DS211+r1";
#elif defined(SYNO_HW_VERSION_RS411)
	return "RS411";
#elif defined(SYNO_HW_VERSION_212)
	return "DS212";
#elif defined(SYNO_HW_VERSION_RS212)
	return "RS212";
#elif defined(SYNO_HW_VERSION_RS812)
	return "RS812";
#elif defined(SYNO_HW_VERSION_212p)
	return "DS212+";
#elif defined(SYNO_HW_VERSION_112j)
	return "DS112j";
#elif defined(SYNO_HW_VERSION_112slim)
	return "DS112slim";
#else
	return "Unknown";
#endif
}

int fan_status_get(int fanno)
{
#if defined(SYNO_HW_VERSION_109) || \
    defined(SYNO_HW_VERSION_RS409) || \
    defined(SYNO_HW_VERSION_RS409r1) || \
    defined(SYNO_HW_VERSION_110j) || \
	defined(SYNO_HW_VERSION_110jr1) || \
	defined(SYNO_HW_VERSION_110jr2) || \
	defined(SYNO_HW_VERSION_110) || \
	defined(SYNO_HW_VERSION_111) || \
	defined(SYNO_HW_VERSION_211p) || \
	defined(SYNO_HW_VERSION_211pr1) || \
    defined(SYNO_HW_VERSION_RS411)|| \
	defined(SYNO_HW_VERSION_212)|| \
	defined(SYNO_HW_VERSION_RS212)|| \
	defined(SYNO_HW_VERSION_RS812)|| \
	defined(SYNO_HW_VERSION_212p)|| \
	defined(SYNO_HW_VERSION_112j)

        int FanStatus;
        char rgcVolt[2] = {0, 0};
        
        do {
                SYNO_CTRL_FAN_STATUS_GET(fanno, &FanStatus);
                rgcVolt[(int)FanStatus] ++;
                if (rgcVolt[0] && rgcVolt[1]) {
                        break;
                }
                udelay(1000);
        } while ( (rgcVolt[0] + rgcVolt[1]) < 200 );

        if ((rgcVolt[0] == 0) || (rgcVolt[1] == 0) ) {
                return 0;
        } else {
                return 1;
        }
#else
        int FanStatus;
 
        SYNO_CTRL_FAN_STATUS_GET(fanno, &FanStatus);

	return FanStatus;
#endif
}

void syno_custom_bootargs_get(char *bootargs)
{
	sprintf(bootargs, CONFIG_CUSTOM_BOOTARGS, SYNO_BOOT_DEV, 
						  SYNO_HW_VERSION,
						  SYNO_POWERON_SEQ,
						  SYNO_NETIF_NUM);
}

void syno_remove_org_model_str(char *szCmd, char *key)
{
    char *ptrStart = szCmd;
    char *ptrEnd = szCmd;
    char szBuf[256];
    char *ptr = szBuf;
    int cKeyLen;

    memset(szBuf, 0, sizeof(szBuf));
    cKeyLen = strlen(key);
    while ( '\0' != *ptrEnd && '\n' != *ptrEnd ) {
        if ( ' ' != *ptrEnd ) {
            ptrEnd++;
            continue;
        }
        *ptrEnd = '\0';
        ptrEnd++;

	if ('\0' != *ptrStart &&
		!( (0 == strncmp(ptrStart, key, cKeyLen)) &&
		    ('=' == ptrStart[cKeyLen] || '\0' == ptrStart[cKeyLen]) ) )
        {
             sprintf(ptr, "%s ", ptrStart);
             ptr += strlen(ptr);
        }
        ptrStart = ptrEnd;
    }

    memset(szCmd, 0, sizeof(szCmd));
    sprintf(szCmd, "%s", szBuf);
}

void syno_custom_bootargs_replace(char *bootargs, char *key, char *value)
{
	char szBuf[128];

	syno_remove_org_model_str(bootargs, key);
	
	sprintf(szBuf, "%s=%s", key, value);
	strcat(bootargs, szBuf);
}

void syno_custom_led_set(int status)
{
	int i = 0;

#if defined(SYNO_GPIO_FOR_1_2_BAY) || defined(SYNO_GPIO_FOR_6282_1_2_BAY)
	for (i=1; i<=SYNO_MAX_DISK; i++) {
		SYNO_CTRL_INTERNAL_HDD_LED_SET(i, status);
	}
#elif defined(SYNO_GPIO_FOR_SLIM) || defined(SYNO_GPIO_FOR_4_5_BAY) || defined(SYNO_GPIO_FOR_6282_SLIM) || defined(SYNO_GPIO_FOR_6282_RS) || defined(SYNO_GPIO_FOR_6282_DS)
	for (i=1; i<=SYNO_MAX_DISK; i++) {
		SYNO_CTRL_EXT_CHIP_HDD_LED_SET(i, status);
	}
#endif
}

int syno_custom_fan_status_get(int *pFanStatus, int size)
{
	int ret = 1;
	int i = 0;

	for ( i = 1; i <= SYNO_MAX_FAN; i++) {
		pFanStatus[i-1] = fan_status_get(i);
		if ( pFanStatus[i-1] <= 0 ) {
			ret = 0;
		}
	}

	return ret;
}

#define I2C_RTC_ADDR    0x32
char * get_random_ethaddr(int idx)
{
	static char ethaddr[17];
	char buf[4];
	i2c_read(I2C_RTC_ADDR, 0, 1, buf, 4);
	sprintf(ethaddr, "00:%02x:%02x:%02x:%02x:%02x", buf[3]+idx, buf[2], buf[1], buf[0], idx);
	return ethaddr;
}

#endif
