/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "types.h"
#include "hw_icoll.h"
#include "hw_icoll_internal.h"
#include "registers/regsicoll.h"


////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
void hw_icoll_SoftTrigger(ICOLL_IRQ_enums_t eIrqVectorNumber, bool bEnable)   
{
    if(bEnable)
        HW_ICOLL_INTERRUPTn_SET(eIrqVectorNumber, BM_ICOLL_INTERRUPTn_SOFTIRQ);
    else
        HW_ICOLL_INTERRUPTn_CLR(eIrqVectorNumber, BM_ICOLL_INTERRUPTn_SOFTIRQ);
}   
      
void hw_icoll_SetFiqMode(ICOLL_FIQ_enums_t eIrqVectorNumber, bool bEnable)
{
    if(bEnable)
        HW_ICOLL_INTERRUPTn_SET(eIrqVectorNumber, BM_ICOLL_INTERRUPTn_ENFIQ);
    else
        HW_ICOLL_INTERRUPTn_CLR(eIrqVectorNumber, BM_ICOLL_INTERRUPTn_ENFIQ);
}

bool hw_icoll_CtrlRegisterUpdate(IcollCtrlRegisterEnums_t ControlRegisterField, bool bSet)
{
    uint32_t u32_icollControl = HW_ICOLL_CTRL_RD();

    if(bSet)
        HW_ICOLL_CTRL_SET(1<<ControlRegisterField);
    else
        HW_ICOLL_CTRL_CLR(1<<ControlRegisterField);

    return (u32_icollControl >> ControlRegisterField) & 0x00000001;
}

////////////////////////////////////////////////////////////////////////////////
// End of file
////////////////////////////////////////////////////////////////////////////////
//! @}
