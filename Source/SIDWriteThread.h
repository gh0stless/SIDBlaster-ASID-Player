/*
  ==============================================================================

    SIDWriteThread.h
    Created: 22 Oct 2024 10:28:42am
    Author:  andre

  ==============================================================================
*/
#pragma once

class SIDWriteThread : public juce::Thread {

public:
    SIDWriteThread(ThreadSafeRingBuffer<WriteSet>& buffer, int deviceID)
        : Thread("SIDWriteThread"), ringBuffer(buffer), device(deviceID) {}
    void run() override {

        switch (device) {
        case 0: setPriority(juce::Thread::Priority::highest);
                break;
        case 1: setPriority(juce::Thread::Priority::high);
            break;
        case 2: setPriority(juce::Thread::Priority::normal);
            break;
        }
        while (!threadShouldExit()) {
            WriteSet value;
            int RS = 0;
            if (ringBuffer.remove(value)) {
                for (int cycles = 0; cycles <= (FRAME_IN_CYCLES / 8)/4; cycles += 8) {
                    if (ringBuffer.size() == 0 || cycles == (FRAME_IN_CYCLES / 8)/4 - 8) {
                        RS = 0;
                        while (RS != HSID_USB_WSTATE_OK) {
                            RS = HardSID_Try_Write(device, FRAME_IN_CYCLES - cycles, value.SIDRegister, value.SIDData);
                            if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        }
                        goto exit_Loop;
                    }
                    RS = 0;
                    while (RS != HSID_USB_WSTATE_OK) {
                        RS = HardSID_Try_Write(device, cycles, value.SIDRegister, value.SIDData);
                        if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    }
                    ringBuffer.remove(value);
                }
            }
            else { // wenn der Puffer leer ist
                RS = 0;
                while (RS != HSID_USB_WSTATE_OK) {
                    RS = HardSID_Try_Write(device, FRAME_IN_CYCLES, 0x1e, 0);  // mache nichts
                    if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }
        exit_Loop:;
        }
    }

private:
    ThreadSafeRingBuffer<WriteSet>& ringBuffer;
    int device; 
};



//#pragma once
//
//
//// Der Verbraucherthread, der Daten aus dem Ringpuffer entnimmt
//class SIDWriteThread : public juce::Thread {
//public:
//    SIDWriteThread(ThreadSafeRingBuffer<WriteSet>& buffer) : Thread("SIDWriteThread"), ringBuffer0(buffer) {}
//
//    void run() override {
//        while (!threadShouldExit()) {
//            WriteSet value;
//            int RS = 0;
//            if (ringBuffer0.remove(value)) {
//                
//                // Schreib Puffer
//                
//                for (int cycles = 0; cycles <= FRAME_IN_CYCLES / 8; cycles+8) {
//                    if (ringBuffer0.size() == 0 || cycles == FRAME_IN_CYCLES / 8) {
//                        RS = 0;
//                        while (RS != HSID_USB_WSTATE_OK) {
//                            RS = HardSID_Try_Write(0, FRAME_IN_CYCLES - cycles, value.SIDRegister, value.SIDData);
//                            if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
//                        }
//                        goto exit_Loop;
//                    }
//                    RS = 0;
//                    while (RS != HSID_USB_WSTATE_OK) {
//                        RS = HardSID_Try_Write(0, cycles, value.SIDRegister, value.SIDData);
//                        if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
//                    }
//                    ringBuffer0.remove(value);
//                }
//            }
//            else { // wenn der Puffer leer ist
//                RS = 0;
//                while (RS != HSID_USB_WSTATE_OK) {
//                    RS = HardSID_Try_Write(0, FRAME_IN_CYCLES, 0x1e, 0);  // mache nichts
//                    if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(20));
//                }
//            }
//        exit_Loop:;
//        }
//    }
//private:
//    ThreadSafeRingBuffer<WriteSet>& ringBuffer0;
//};