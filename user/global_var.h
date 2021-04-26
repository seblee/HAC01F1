#ifndef __GLOBAL_VAR_H
#define __GLOBAL_VAR_H
#include "sys_def.h"
#include "sys_conf.h"

#define VALVE1_STEP_MANID   	28u
#define VALVE2_STEP_MANID   	29u 
#define FACTORY_RESET		    15u	

extern sys_reg_st					g_sys;

uint16_t save_conf_reg(uint8_t addr_sel);
uint16_t set_load_flag(uint8_t ee_load_flag);
uint16_t sys_global_var_init(void);
uint16_t sys_local_var_init(void);
int16_t eeprom_tripple_write(uint16_t reg_offset_addr,uint16_t wr_data,uint16_t rd_data);

int16_t eeprom_compare_read(uint16_t reg_offset_addr, uint16_t *rd_data);

uint16_t reg_map_write(uint16_t reg_addr,uint16_t *wr_data, uint8_t wr_cnt);
uint8_t load_factory_pram(void);
uint8_t fan_com_chk(uint16_t pram);
#endif /*__GLOBAL_VAR_H*/
