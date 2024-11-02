/*
  ==============================================================================

    Sid.cpp
    Created: 31 Oct 2017 5:48:50pm
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/

#include "Sid.h"
//#include "MainComponent.h"
//------------------------------------------------------------------------------




Sid::Sid() : ringBuffer0(),
ringBuffer1(),
ringBuffer2(),
playerThread(ringBuffer0,
	ringBuffer1,
	ringBuffer2,
	//noOfPlayingDevicesMutex,
	No_Of_Playing_Devices)
{
	#if defined(_WIN32) || defined(_WIN64)
		// Ermitteln des Programmverzeichnisses
		juce::File programDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();

		// Pfad zur Bibliothek konstruieren
		juce::File libPath = programDirectory.getChildFile("hardsid.dll");

		hardsiddll = hardsidlibrary.open(libPath.getFullPathName());
		#endif	

		#if defined(__linux)
		hardsiddll = hardsidlibrary.open("/usr/local/lib/libhardsid.so");
		#endif

		#if defined(__APPLE__)
		hardsiddll = hardsidlibrary.open("/usr/local/lib/libhardsid.dylib");
		#endif

		// Check to see if the library was loaded successfully 
		if (hardsiddll == true) {
          
            My_HardSID_Version = (lpHardSID_Version)hardsidlibrary.getFunction("HardSID_Version");
            My_HardSID_Devices = (lpHardSID_Devices)hardsidlibrary.getFunction("HardSID_Devices");
            My_HardSID_Flush = (lpHardSID_Flush)hardsidlibrary.getFunction("HardSID_Flush");
            My_HardSID_SoftFlush = (lpHardSID_SoftFlush)hardsidlibrary.getFunction("HardSID_SoftFlush");
            My_HardSID_Lock = (lpHardSID_Lock)hardsidlibrary.getFunction("HardSID_Lock");
            My_HardSID_Reset = (lpHardSID_Reset)hardsidlibrary.getFunction("HardSID_Reset");          
            My_HardSID_Try_Write = (lpHardSID_Try_Write)hardsidlibrary.getFunction("HardSID_Try_Write");
            My_HardSID_Uninitialize   = (lpHardSID_Uninitialize)hardsidlibrary.getFunction("HardSID_Uninitialize");
			My_HardSID_GetSIDType = (lpHardSID_GetSIDType)hardsidlibrary.getFunction("HardSID_GetSIDType");

			Number_Of_Devices = (int)My_HardSID_Devices();
			if (Number_Of_Devices == 0) {
				error_state = 2;
			}
		}
		else error_state = 1;
}

Sid::~Sid() {
#if defined(__APPLE__) || defined(__linux__)
	My_HardSID_Uninitialize();
#endif
}

//------------------------------------------------------------------------------

	int Sid::GetDLLVersion(void) {
		return (int)My_HardSID_Version();
	}

	int Sid::GetSidType(int device) {
		return	My_HardSID_GetSIDType(device);
	}

	void Sid::init(int device) {
		if (!error_state) {

			//Init SID
			My_HardSID_Lock(device);
			My_HardSID_Reset(device);
			My_HardSID_SoftFlush(device);
			My_HardSID_Flush(device);
			//Init Registers
			push_event(device, 0, 0x00);
			juce::Thread::sleep(300);
			for (auto r = 0; r < NUMSIDREGS; r++) {
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

	void Sid::startPlayerThread(void) {
			if (!playerThread.isThreadRunning()) {
				playerThread.startThread();
			}
		
	}

	void Sid::stopPlayerThread(void) {

			if (playerThread.isThreadRunning()) {
				playerThread.signalThreadShouldExit();
				playerThread.waitForThreadToExit(1000);
			}
	}

	bool Sid::isPlayerThreadRuning(void) {
		return playerThread.isThreadRunning();
	}
