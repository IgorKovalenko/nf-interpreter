//
// Copyright (c) 2017 The nanoFramework project contributors
// Portions Copyright (c) Microsoft Corporation.  All rights reserved.
// See LICENSE file in the project root for full license information.
//
#ifndef _NANOCLR_HARDWARE_H_
#define _NANOCLR_HARDWARE_H_

#include <nanoCLR_Types.h>
#include <nanoCLR_Runtime.h>
//#include <nanoCLR_Graphics.h>

//--//

//
// THIS HAS TO MATCH THE ENUM Microsoft.SPOT.Hardware.ButtonCode!!!!
//
// UNDONE: FIXME: static const int c_CLR_HW_Button_Up        = 0;
//static const int c_CLR_HW_Button_Down      = 1;
//static const int c_CLR_HW_Button_Spare     = 2;
//static const int c_CLR_HW_Button_Select    = 3;
//static const int c_CLR_HW_Button_Channel   = 4;
//static const int c_CLR_HW_Button_Backlight = 5;
//
//static const int c_CLR_HW_Button_Raw_B0    = 10;
//static const int c_CLR_HW_Button_Raw_B1    = 11;
//static const int c_CLR_HW_Button_Raw_B2    = 12;
//static const int c_CLR_HW_Button_Raw_B3    = 13;
//static const int c_CLR_HW_Button_Raw_B4    = 14;
//static const int c_CLR_HW_Button_Raw_B5    = 15;
//static const int c_CLR_HW_Button_Raw_B6    = 16;
//static const int c_CLR_HW_Button_Raw_B7    = 17;
//static const int c_CLR_HW_Button_Raw_B8    = 18;
//static const int c_CLR_HW_Button_Raw_B9    = 19;
//
//static const int c_CLR_HW_Button_MAX       = 32;

#if defined(_WIN32)
#include "nanoCLR_Win32.h"
#endif

//--//

struct CLR_HW_Hardware
{
    enum PowerLevel
    {
        PowerLevel__Active,
        PowerLevel__SelectiveOff,
        PowerLevel__Sleep,
        PowerLevel__DeepSleep,
        PowerLevel__Off
    };

    //--//
   
    static const CLR_UINT32 c_Default_PowerLevel   = PowerLevel__Sleep; 
    static const CLR_UINT32 c_Default_WakeupEvents = SYSTEM_EVENT_FLAG_COM_IN            |
                                                     SYSTEM_EVENT_FLAG_COM_OUT           |
                                                     SYSTEM_EVENT_FLAG_USB_IN            |
                                                     SYSTEM_EVENT_FLAG_SYSTEM_TIMER      |
                                                     SYSTEM_EVENT_HW_INTERRUPT           |
                                                     // UNDONE: FIXME: SYSTEM_EVENT_I2C_XACTION            |
                                                     SYSTEM_EVENT_FLAG_SOCKET            |
                                                     // UNDONE: FIXME: SYSTEM_EVENT_FLAG_CHARGER_CHANGE    |
                                                     SYSTEM_EVENT_FLAG_DEBUGGER_ACTIVITY |
                                                     SYSTEM_EVENT_FLAG_MESSAGING_ACTIVITY|
                                                     SYSTEM_EVENT_FLAG_IO                ;

    //--//

    struct HalInterruptRecord
    {
        CLR_INT64              m_time;
        CLR_RT_HeapBlock_Node* m_context;

        CLR_UINT32             m_data1;
        CLR_UINT32             m_data2;
        CLR_UINT32             m_data3;
    
        void AssignFrom( const HalInterruptRecord &recFrom )
        {
            m_time      = recFrom.m_time;
            m_context   = recFrom.m_context;
            m_data1     = recFrom.m_data1;
            m_data2     = recFrom.m_data2;
        }
    };

    struct InterruptData
    {
        Hal_Queue_UnknownSize<HalInterruptRecord> m_HalQueue;
        CLR_RT_DblLinkedList                      m_applicationQueue;
        CLR_UINT32                                m_queuedInterrupts;
    };

    //--//

    bool          m_fInitialized;

    InterruptData m_interruptData;

    CLR_UINT32    m_DebuggerEventsMask;
    CLR_UINT32    m_MessagingEventsMask;

    CLR_UINT32    m_powerLevel;
    CLR_UINT32    m_wakeupEvents;
    
    //--//

    __nfweak static HRESULT CreateInstance();

    __nfweak HRESULT Hardware_Initialize();

    __nfweak static HRESULT DeleteInstance();

    __nfweak void Hardware_Cleanup();

    __nfweak void PrepareForGC   ();
    __nfweak void ProcessActivity();

    __nfweak HRESULT ManagedHardware_Initialize();
    __nfweak HRESULT ProcessInterrupts();
    __nfweak HRESULT SpawnDispatcher();
    __nfweak HRESULT TransferAllInterruptsToApplicationQueue();

    // UNDONE: FIXME: void Screen_Flush( CLR_GFX_Bitmap& bitmap, CLR_UINT16 x, CLR_UINT16 y, CLR_UINT16 width, CLR_UINT16 height );
};

extern CLR_HW_Hardware g_CLR_HW_Hardware;

//--//


////////////////////////////////////////////////////////////////////////////////////////
// keep under control the size of the HalInterruptRecord, since we will use externally
// defined arrays to handle those data structures in the interrupt dispatching

CT_ASSERT(sizeof(CLR_HW_Hardware::HalInterruptRecord) == 24)

////////////////////////////////////////////////////////////////////////////////////////

//--//

#endif // _NANOCLR_HARDWARE_H_

