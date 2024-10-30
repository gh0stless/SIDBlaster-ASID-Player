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
    SIDWriteThread(ThreadSafeRingBuffer<SIDWriteSet>& buffer0, ThreadSafeRingBuffer<SIDWriteSet>& buffer1, ThreadSafeRingBuffer<SIDWriteSet>& buffer2, int& noofdevices)
        : Thread("SIDWriteThread"), ringBuffer0(buffer0), ringBuffer1(buffer1), ringBuffer2(buffer2), NoOfDevices(noofdevices) {}
    void run() override {
        setPriority(juce::Thread::Priority::highest);
        while (!threadShouldExit()) {
           
            for (auto i = 0; i < NoOfDevices; i++) {
                bool cie = false;
                switch (i) {
                case 0:  cie = ringBuffer0.remove(value);
                    break;
                case 1:  cie = ringBuffer1.remove(value);
                    break;
                case 2:  cie = ringBuffer2.remove(value);
                    break;
                }
                if (cie) {
                    HardSID_WriteWithTimeout(i, cycles, value.SIDRegister, value.SIDData);
                }
                else { // wenn der Puffer leer ist
                    HardSID_WriteWithTimeout(i, cycles, 0x1e, 0); //mache nichts
                }
                //juce::Thread::sleep(1);  // Kleiner Sleep, um die CPU-Belastung zu senken
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
    const int LOOP_TIME_OUT_MILLIS = 500;  // Timeout in Millisekunden
    const int cycles = 8 ;
    SIDWriteSet value ;
    ThreadSafeRingBuffer<SIDWriteSet>& ringBuffer0;
    ThreadSafeRingBuffer<SIDWriteSet>& ringBuffer1;
    ThreadSafeRingBuffer<SIDWriteSet>& ringBuffer2;
    int& NoOfDevices;
};
