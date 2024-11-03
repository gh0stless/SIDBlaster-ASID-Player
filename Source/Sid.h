/*
  ==============================================================================

    Sid.h
    Created: 31 Oct 2017 5:48:50pm
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "hardsid.h"
#include "Mutex.h"

struct SIDWriteSet {
	Uint8 SIDRegister;
	Uint8 SIDData;
};

enum SID_TYPE {
	SID_TYPE_NONE = 0, SID_TYPE_6581, SID_TYPE_8580
};

#define NUMSIDREGS 0x18 // numbers of (writable) SID-registers
#define SIDWRITEDELAY 14 // lda $xxxx,x 4 cycles, sta $d400,x 5 cycles, dex 2 cycles, bpl 3 cycles
#define HARDSID_FLUSH_CYCLES 1000
#define HARDSID_FLUSH_MS 50
#define PAL_FRAMERATE 50
#define PAL_CLOCKRATE 985248
#define NTSC_FRAMERATE 60
#define NTSC_CLOCKRATE 1022727 //This is for machines with 6567R8 VIC. 6567R56A is slightly different.
#define FRAME_IN_CYCLES 19705 //( 17734475 / 18 / 50 )   // 50Hz in cycles for PAL clock

#define MY_BUFFER_SIZE 8192*2

#include "ThreadSafeRingBuffer.h"
#include "SIDWriteThread.h"
//#include <atomic>

//==============================================================================

class Sid {
	public:
		Sid();
		~Sid();
		int GetDLLVersion(void);
		int GetSidType(int device);
		void init(int device);
		void push_event(int device, Uint8 reg, Uint8 val);
		void startPlayerThread(void);
		void stopPlayerThread(void);
		bool isPlayerThreadRuning(void);

		int error_state = 0;
		int Number_Of_Devices = 0;
		//int Number_Of_Playing_Devices = 0;
		//int No_Of_Playing_Devices = 0;
	private:	

        typedef Uint16  (*lpHardSID_Version)(void);
		typedef Uint8   (*lpHardSID_Devices)(void);
		typedef void    (*lpHardSID_Flush)(Uint8 DeviceID);
		typedef void    (*lpHardSID_SoftFlush)(Uint8 DeviceID);
		typedef bool    (*lpHardSID_Lock)(Uint8 DeviceID);
		typedef void    (*lpHardSID_Reset)(Uint8 DeviceID);
		typedef Uint8   (*lpHardSID_Try_Write)(Uint8 DeviceID, Uint16 Cycles, Uint8 SID_reg, Uint8 Data);
	    typedef void    (*lpHardSID_Uninitialize)(void);
		typedef int 	(*lpHardSID_GetSIDType)(Uint8 DeviceID);

		lpHardSID_Version My_HardSID_Version = nullptr;
		lpHardSID_Devices My_HardSID_Devices = nullptr;
		lpHardSID_Flush My_HardSID_Flush = nullptr;
		lpHardSID_SoftFlush My_HardSID_SoftFlush = nullptr;
		lpHardSID_Lock My_HardSID_Lock = nullptr;
		lpHardSID_Reset My_HardSID_Reset = nullptr;
		lpHardSID_Try_Write My_HardSID_Try_Write = nullptr;
		lpHardSID_Uninitialize My_HardSID_Uninitialize = nullptr;
		lpHardSID_GetSIDType My_HardSID_GetSIDType = nullptr;

		juce::DynamicLibrary hardsidlibrary;

		bool hardsiddll = false;
		
		ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE> ringBuffer0;
		ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE> ringBuffer1;
		ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE> ringBuffer2;
		SIDWriteThread playerThread;  

};
