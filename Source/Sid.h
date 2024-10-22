/*
  ==============================================================================

    Sid.h
    Created: 31 Oct 2017 5:48:50pm
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/

#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#include "../JuceLibraryCode/JuceHeader.h"
#include "./hardsid.h"

#define NUMSIDREGS 0x18 // numbers of (writable) SID-registers
#define SIDWRITEDELAY 14 // lda $xxxx,x 4 cycles, sta $d400,x 5 cycles, dex 2 cycles, bpl 3 cycles
#define HARDSID_FLUSH_CYCLES 1000
#define HARDSID_FLUSH_MS 50
#define PAL_FRAMERATE 50
#define PAL_CLOCKRATE 985248
#define NTSC_FRAMERATE 60
#define NTSC_CLOCKRATE 1022727 //This is for machines with 6567R8 VIC. 6567R56A is slightly different.
#define FRAME_IN_CYCLES 19705 //( 17734475 / 18 / 50 )   // 50Hz in cycles for PAL clock
#define MY_BUFFER_SIZE 10000

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned char  BYTE;

enum SID_TYPE {
	SID_TYPE_NONE = 0, SID_TYPE_6581, SID_TYPE_8580
};

class Sid {
	public:
		Sid();
		~Sid();
		int GetDLLVersion(void);
		int Sid::GetSidType(int device);
		void init(int device);
		void push_event(int device, Uint8 reg, Uint8 val);

		int error_state = 0;
		int Number_Of_Devices = 0;

	private:	
		juce::DynamicLibrary hardsidlibrary;

};