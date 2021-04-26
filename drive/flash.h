#ifndef __FLASH_H
#define	__FLASH_H

#include "stm32f0xx.h"
/*FLASH definition*/
#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x08003c00)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x08004000)   /* End @ of user Flash area */
#define FLASH_P_FLAG						((uint16_t)0x1bdf)				/*flash programed flag*/

int16_t flash_write(uint32_t addr, const uint16_t* p_data, uint16_t count);
#endif/*__FLASH_H*/

