/*
  ==============================================================================

    Sid.cpp
    Created: 31 Oct 2017 5:48:50pm
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/

#include "Sid.h"
#include <chrono>
#include <thread>

//------------------------------------------------------------------------------

Sid::Sid() {
		Number_Of_Devices = (int)HardSID_Devices();
		if (Number_Of_Devices == 0) {
				error_state = 3;
			}
	}

Sid::~Sid() {
#if defined(__APPLE__) || defined(__linux__)
	HardSID_Uninitialize();
#endif
}

//------------------------------------------------------------------------------
	int Sid::GetDLLVersion(void) {
		return (int)HardSID_Version();
	}
	int Sid::GetSidType(int device) {
		return	HardSID_GetSIDType(device);
	}
	void Sid::init(int device) {
		if (!error_state) {

			//Init SID
			HardSID_Reset(device);
			HardSID_Lock(device);
			HardSID_Flush(device);

			//Init Registers
			push_event(device, 0, 0x00);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			BYTE r;
			for (r = 0; r <= NUMSIDREGS; r++) {
				push_event(device, r, 0x00);
			}
		}
	}
	void Sid::push_event(int device, Uint8 reg, Uint8 val) {

		Uint8 RS = 0;
		if (!error_state) {
			while (RS != HSID_USB_WSTATE_OK)
			{
				RS = HardSID_Try_Write(device, 0, reg, val);
				if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
			HardSID_SoftFlush(device);
		}
	}

