/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (c) 2014 Alex Winter (eterno.despierto@gmail.com)
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */


#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include "ctypes.h"


/************************************************
 *              COMMON  TYPES DEFINITIONS                       *
 ************************************************/

//***** Define unsigned 8-bit byte type *****
#ifndef _U8_
#define _U8_
   typedef unsigned char                    U8, *PU8;
   typedef volatile unsigned char           V8, *PV8;
#endif

//***** Define signed 8-bit byte type *****
#ifndef _S8_
    #define _S8_
    typedef signed char                     S8, *PS8;
#endif

//***** Define unsigned 16-bit word type *****
#ifndef _U16_
    #define _U16_
    typedef unsigned short                  U16, *PU16;
    typedef volatile unsigned short         V16, *PV16;
#endif

//***** Define signed 16-bit word type *****
#ifndef _S16_
    #define _S16_
    typedef signed short                    S16, *PS16;
#endif

//***** Define unsigned 32-bit word type *****
#ifndef _U32_
    #define _U32_
    typedef unsigned int                    U32, *PU32;
    typedef volatile unsigned int           V32, *PV32;
#endif

//***** Define signed 32-bit word type *****
#ifndef _S32_
    #define _S32_
    typedef signed int                      S32, *PS32;
#endif

//***** Define unsigned 64-bit word type *****
#ifndef _U64_
    #define _U64_
    typedef unsigned long long              U64, *PU64;
    typedef volatile unsigned long long     V64, *PV64;
#endif

//***** Define signed 64-bit word type *****
#ifndef _S64_
    #define _S64_
    typedef signed long long                S64, *PS64;
#endif

//***** Define unsigned 40-bit word type *****
#ifndef _U40_
    #define _U40_
    typedef double                          U40, *PU40;
#endif

//***** Define signed 40-bit word type *****
#ifndef _S40_
    #define _S40_
    typedef double                          S40, *PS40;
#endif

#ifndef _PTR_
    #define _PTR_
    typedef void                            *PTR;
    typedef volatile void                   *VPTR;    
#endif



/************************************************
 *              EXTENDED  TYPES DEFINITIONS                     *
 ************************************************/

//***** Define bit field *****
#ifndef _BIT_FIELD_
    #define _BIT_FIELD_
    typedef unsigned int                    BIT_FIELD;
#endif

//***** Define NULL PTR *****
#ifndef NULL
    #define NULL   (void*)0
#endif

//***** Define Inlide *****
#ifndef __INLINE__
    #define __INLINE__
    #define INLINE                          inline
#endif

//***** Define boolean *****
#ifndef _BOOL_
    #define _BOOL_
    typedef U32                             BOOL;
    //typedef bool                                                  BOOL;
#endif

#ifndef _BOOLEAN_
    #define _BOOLEAN_
    typedef U8                              BOOLEAN;
#endif

//***** Define handle *****
#ifndef _HANDLE_
    #define _HANDLE_
    typedef void                            *HANDLE;
#endif

//***** Result code *****
#ifndef _RESULTCODE_
    #define _RESULTCODE_
    typedef int                             RESULTCODE;
#endif




/************************************************
 *              SPECIAL  TYPES DEFINITIONS                     *
 ************************************************/
    
//***** Define IP address type *****
#ifndef _IPADDR_
    #define _IPADDR_
    typedef unsigned int                    IPaddr_t;
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef int                                 ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int                        size_t;
#endif



/************************************************
 *              ALTERNATIVE  TYPES DEFINITIONS                     *
 ************************************************/

//! \brief TBD
//! \todo [PUBS] Add definition(s)...
//! \todo Where does this really go?
typedef struct
{
    //! \brief TBD
    uint32_t val[4];
} uint128_t;

//! \brief TBD
#ifndef RETCODE
#define RETCODE int
#endif

//------------------------------------------------------------------------------
// All of the following defs are included for compatability.  Please use the
// ANSI C99 defs for all new work.
//------------------------------------------------------------------------------

//! \brief TBD
#if !defined(FALSE)
#define FALSE false
#endif

//! \brief TBD
#if !defined(TRUE)
#define TRUE  true
#endif

//! \brief TBD
#if !defined(NULL)
#define NULL 0
#endif

//! \brief TBD
typedef uint8_t     UINT8;
//! \brief TBD
typedef uint8_t     BYTE;
//! \brief TBD
typedef uint8_t     PACKED_BYTE;

//! \brief TBD
typedef uint16_t    UINT16;
//! \brief TBD
typedef uint16_t    USHORT;
//! \brief TBD
typedef uint16_t    WCHAR;
//! \brief TBD
typedef uint16_t    UCS3;
//! \brief TBD
typedef int16_t     SHORT;

//! \brief TBD
typedef uint32_t    UINT32;
//! \brief TBD
typedef uint32_t    WORD;
//! \brief TBD
typedef uint32_t    SECTOR_BUFFER;
//! \brief TBD
typedef uint32_t *  P_SECTOR_BUFFER;

//! \brief TBD
typedef uint64_t    DWORD;
//! \brief TBD
typedef int64_t     INT64;
//! \brief TBD
typedef int64_t     UINT64;

//! \brief TBD
typedef uint128_t   UINT128;

//! \brief TBD
typedef float       FLOAT;

//! \brief TBD
typedef unsigned char   uchar;

/* sysv */
typedef unsigned char   unchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

#ifndef __nop
    #define __nop() asm volatile ("mov r0,r0" ::)
    #define __NOP()    
#endif


#endif /* __TYPES_H__ */

