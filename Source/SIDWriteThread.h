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

        //setPriority(juce::Thread::Priority::background);

        while (!threadShouldExit()) {
            
            WriteSet value;
            int RS = 0;
            if (ringBuffer.remove(value)) {
            #define WRITES_PER_FRAME 64 * 8
                for (int cycles = 0; cycles < WRITES_PER_FRAME; cycles += 8) {
                    if (ringBuffer.size() == 0 || cycles == WRITES_PER_FRAME-8) {
                        RS = 0;
                        while (RS != HSID_USB_WSTATE_OK) { //letzter write
                            RS = HardSID_Try_Write(device, FRAME_IN_CYCLES - cycles, value.SIDRegister, value.SIDData);
                            if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        }
                        goto exit_Loop;
                    }
                    RS = 0;
                    while (RS != HSID_USB_WSTATE_OK) {//normaler write
                        RS = HardSID_Try_Write(device, cycles, value.SIDRegister, value.SIDData);
                        if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    }
                    ringBuffer.remove(value);
                }
                
            }
            else { // wenn der Puffer leer ist
                RS = 0;
                while (RS != HSID_USB_WSTATE_OK) {
                    RS = HardSID_Try_Write(device, FRAME_IN_CYCLES, 0x1e, 0);  // mache nichts
                    if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        exit_Loop:;
        }
    }

private:
    ThreadSafeRingBuffer<WriteSet>& ringBuffer;
    int device; 
};
