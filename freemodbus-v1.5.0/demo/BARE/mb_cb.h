#ifndef __MB_CB_H
#define	__MB_CB_H
//modbus holding regs
#include "stdint.h"
#include "port.h"
//alarm definition
#define ALARM_WQ_TH				250
#define ALARM_HC_TH				10000
enum
{
    BAUD_4800 = 0,
	BAUD_9600,
	BAUD_19200,
	BAUD_38400,
	BAUD_57600,
	BAUD_115200
};

//modbus related
#define REG_HOLDING_START 0
#define REG_HOLDING_NREGS 256

#define CONFIG_REG_MAP_OFFSET  0
#define STATUS_REG_MAP_OFFSET  110
#define CPAD_REG_HOLDING_WRITE_NREGS (STATUS_REG_MAP_OFFSET + 1)

#define MB_SOFTWARE_VER					0x01 
#define MB_SOFTWARE_SUBVER 			0x00
#define MB_HARDWARE_VER					0x02
#define MB_HARDWARE_SUBVER 			0x00
#define MB_DEVICE_TYPE					"S01H"
#define MB_SERIAL_NO						"11223344"
#define MB_MAN_DATE							"20151014"
#define MB_DEVICE_ADDR					9
//#define MB_DEVICE_ADDR					11//用于电流检测用
#define MB_BAUDRATE							4800

#define CMD_MB_SAVE_FLASH				4
#define CMD_MB_SYS_RESET				3
#define CMD_MB_RESET_DEFAULT		2
#define CMD_MB_FACTORY_MODE			1
#define CMD_MB_USER_MODE				0

#define MB_FLASH_WR_FLAG				0x1bdf



void mb_reg_update(void);
//void mb_cmd_resolve(void);
ULONG mb_get_baudrate(uint16_t baudrate);
uint8_t mb_get_device_addr(void);
#endif/*__MB_CB_H*/
