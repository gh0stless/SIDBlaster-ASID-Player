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
    SIDWriteThread(ThreadSafeRingBuffer<WriteSet>& buffer0, ThreadSafeRingBuffer<WriteSet>& buffer1, ThreadSafeRingBuffer<WriteSet>& buffer2, int& noofdevices)
        : Thread("SIDWriteThread"), ringBuffer0(buffer0), ringBuffer1(buffer1), ringBuffer2(buffer2), NoOfDevices(noofdevices) {}
    void run() override {
        //setPriority(juce::Thread::Priority::background);
        while (!threadShouldExit()) {
            #define WRITES_PER_FRAME 64 * 8
                for (int i = 0; i < NoOfDevices; i++) {
                    int RS = 0;
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
                            int buffersize = 0;
                            switch (i) {
                            case 0:  buffersize = ringBuffer0.size();
                                break;
                            case 1:  buffersize = ringBuffer1.size();
                                break;
                            case 2:  buffersize = ringBuffer2.size();
                                break;
                            }
                            if (buffersize == 0 || cycles[i] == WRITES_PER_FRAME - 8) {
                                RS = 0;
                                while (RS != HSID_USB_WSTATE_OK) { //letzter write
                                    RS = HardSID_Try_Write(i, FRAME_IN_CYCLES - cycles[i], value.SIDRegister, value.SIDData);
                                    if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(2));
                                }
                                cycles[i] = 0;
                            }
                            else {
                                RS = 0;
                                while (RS != HSID_USB_WSTATE_OK) {//normaler write
                                    RS = HardSID_Try_Write(i, cycles[i], value.SIDRegister, value.SIDData);
                                    if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(2));
                                }
                                cycles[i] = cycles[i]+8;
                            }
                    }
                    else { // wenn der Puffer leer ist
                        RS = 0;
                        while (RS != HSID_USB_WSTATE_OK) {
                            RS = HardSID_Try_Write(i, FRAME_IN_CYCLES, 0x1e, 0);  // mache nichts
                            if (RS == HSID_USB_WSTATE_BUSY) std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    }
                }
        }
    }

private:
    ThreadSafeRingBuffer<WriteSet>& ringBuffer0;
    ThreadSafeRingBuffer<WriteSet>& ringBuffer1;
    ThreadSafeRingBuffer<WriteSet>& ringBuffer2;
    int& NoOfDevices; 
    int cycles[3] = { 0, 0, 0 };
    WriteSet value;
};
