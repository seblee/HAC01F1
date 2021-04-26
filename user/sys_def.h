#ifndef __SYS_DEF_H
#define	__SYS_DEF_H

#include "stdint.h"

/*===== 0bit base types  ====================*/
#define VOID                void

/*===== 8bit base types  ====================*/
typedef   signed char       CHAR;
typedef unsigned char       UCHAR;
typedef          UCHAR      BYTE;


/*===== 16bit base types  ===================*/
typedef   signed short      SHORT;
typedef unsigned short      USHORT;
typedef          USHORT     WORD;


/*===== 32bit base types  ===================*/
typedef unsigned long       DWORD;
typedef unsigned long       *HANDLE;


/*===== float base types  ===================*/
typedef          float      FLOAT32;
typedef         double      FLOAT64;



/*===== integer base types  =============*/
typedef          CHAR       INT8;
typedef          UCHAR      UINT8;
typedef          SHORT      INT16;
typedef          USHORT     UINT16;
typedef signed long         INT32;
typedef unsigned long       UINT32;

///*===== Boolean types  ======================*/
//typedef         INT32		BOOL;
#define         FALSE       0
#define         TRUE        1

/*===== Data storage flags ===================*/
#define EXTERN		extern		// import global data/function
#define GLOBAL					// export/define global data/function
#define LOCAL		static		// static
#define VOLATILE	volatile
#define INLINE		inline		// inline api		
#define NOINIT		__no_init

#define CONST		const
#define IN						// input arg
#define OUT						// output arg



#define STATUS_REG_MAP_NUM			    70
#define CONF_REG_MAP_NUM				100

#define SOFTWARE_VER						0x1002
#define HARDWARE_VER						0x1000
#define SERIAL_NO_3							0
#define SERIAL_NO_2							0
#define SERIAL_NO_1							0
#define SERIAL_NO_0							0
#define MAN_DATA_1							0x2017
#define MAN_DATA_0							0x0908
//#define PERM_PRIVILEGED					 1
//#define PERM_INSPECT						0


#endif/*__SYS_DEF_H*/

