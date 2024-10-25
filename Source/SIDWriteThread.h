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
    SIDWriteThread(ThreadSafeRingBuffer<WriteSet>& buffer)
        : Thread("SIDWriteThread"), ringBuffer(buffer) {}
    void run() override {
        //setPriority(juce::Thread::Priority::highest);
        while (!threadShouldExit()) {
    
            #define WRITES_PER_FRAME 64 * 8
                
            int RS = 0;
            bool cie = false;
            cie = ringBuffer.remove(value);
                if (cie) {
                    int buffersize = 0;
                    buffersize = ringBuffer.size();

                    if (buffersize == 0 || cycles == WRITES_PER_FRAME - 8) {
                        RS = 0;
                        while (RS != HSID_USB_WSTATE_OK) { //letzter write
                            RS = HardSID_Try_Write(0, FRAME_IN_CYCLES - cycles, value.SIDRegister, value.SIDData);
                            if (RS == HSID_USB_WSTATE_BUSY) juce::Thread::sleep(20);
                        }
                        cycles = 0;
                    }
                    else {
                        RS = 0;
                        while (RS != HSID_USB_WSTATE_OK) {//normaler write
                            RS = HardSID_Try_Write(0, cycles, value.SIDRegister, value.SIDData);
                            if (RS == HSID_USB_WSTATE_BUSY) juce::Thread::sleep(20);
                        }
                        cycles = cycles+8;
                    }
            }
            else { // wenn der Puffer leer ist
                RS = 0;
                while (RS != HSID_USB_WSTATE_OK) {
                    RS = HardSID_Try_Write(0, FRAME_IN_CYCLES, 0x1e, 0);  // mache nichts
                    if (RS == HSID_USB_WSTATE_BUSY) juce::Thread::sleep(20);
                }
                juce::Thread::sleep(20);
            }
                
        }
    }

private:
    ThreadSafeRingBuffer<WriteSet>& ringBuffer;
    int cycles = 0;
    WriteSet value;
};
