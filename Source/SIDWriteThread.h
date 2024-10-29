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
    SIDWriteThread(ThreadSafeRingBuffer<SIDWriteSet>& buffer)
        : Thread("SIDWriteThread"), ringBuffer(buffer) {}
    void run() override {
        //setPriority(juce::Thread::Priority::highest);
        while (!threadShouldExit()) {
            bool cie = false;
            cie = ringBuffer.remove(value);
                if (cie) {
                    HardSID_WriteWithTimeout(0, cycles, value.SIDRegister, value.SIDData);
                }
                else { // wenn der Puffer leer ist
                    HardSID_WriteWithTimeout(0, FRAME_IN_CYCLES, value.SIDRegister, value.SIDData);
                }
        }
    }

    bool HardSID_WriteWithTimeout(int dev_id, int cycles, int reg, int data) {
        
        auto startTime = juce::Time::getMillisecondCounter();
        
        while (HardSID_Try_Write(dev_id, cycles, reg, data) == HSID_USB_WSTATE_BUSY) {
            juce::Thread::yield();  

            auto elapsedTime = juce::Time::getMillisecondCounter() - startTime;
            if (elapsedTime > LOOP_TIME_OUT_MILLIS) {
                return false;  
            }
        }
        return true;  // Schreibvorgang erfolgreich
    }

private:
    int LOOP_TIME_OUT_MILLIS = 500;  // Timeout in Millisekunden
    ThreadSafeRingBuffer<SIDWriteSet>& ringBuffer;
    int cycles = 8;
    SIDWriteSet value;
};
