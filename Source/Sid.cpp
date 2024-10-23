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

Sid::Sid()	:	ringBuffer0(MY_BUFFER_SIZE),
				ringBuffer1(MY_BUFFER_SIZE),
				ringBuffer2(MY_BUFFER_SIZE),
				consumerThread0(ringBuffer0,0), 
				consumerThread1(ringBuffer1,1), 
				consumerThread2(ringBuffer2,2)  
	{
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

		switch(device) {
		case 0: ringBuffer0.add({ reg, val });
			break;
		case 1: ringBuffer1.add({ reg, val });
			break;
		case 2: ringBuffer2.add({ reg, val });
			break;
		}
	}
	void Sid::startConsumer(int WriteThreadNo) {
		switch (WriteThreadNo) {
		case 0:
			if (!consumerThread0.isThreadRunning()) {
				consumerThread0.startThread();
			}
			break;
		case 1:
			if (!consumerThread1.isThreadRunning()) {
				consumerThread1.startThread();
			}
			break;
		case 2:
			if (!consumerThread2.isThreadRunning()) {
				consumerThread2.startThread();
			}
			break;
		}
	}
	void Sid::stopConsumer(int WriteThreadNo) {
		switch (WriteThreadNo) {
		case 0:
			if (consumerThread0.isThreadRunning()) {
				consumerThread0.signalThreadShouldExit();
				consumerThread0.waitForThreadToExit(1000);
			}
			break;
		case 1:
			if (consumerThread1.isThreadRunning()) {
				consumerThread1.signalThreadShouldExit();
				consumerThread1.waitForThreadToExit(1000);
			}
			break;
		case 2:
			if (consumerThread2.isThreadRunning()) {
				consumerThread2.signalThreadShouldExit();
				consumerThread2.waitForThreadToExit(1000);
			}
			break;
		}
	}
