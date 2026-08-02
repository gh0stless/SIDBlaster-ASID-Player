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
    SIDWriteThread( ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE>& buffer0,
                    ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE>& buffer1,
                    ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE>& buffer2,
                    juce::Atomic<int>& noofplayingdevices)
      : Thread("SIDWriteThread"),
        ringBuffer0(buffer0),
        ringBuffer1(buffer1),
        ringBuffer2(buffer2),
        NoOfPlayingDevices(noofplayingdevices) {}

    // EXPERIMENT, REVERTED: tried dropping the "do nothing" keep-alive
    // write (0x1e) for the "buffer empty but a device is playing" case,
    // on the theory that it only existed to compensate for the classic
    // driver's own busy-spin bug (now fixed - see the driver fork's
    // README). Wrong theory: `cycles` below is a FIXED constant (8), not
    // a computed real-time delta - this design keeps the emulated SID
    // clock in sync with wall-clock time purely by calling the driver as
    // often as possible (at ~985kHz PAL, keeping pace needs roughly
    // 985248/8 ~= 123k calls/sec). The tight loop - keep-alive writes
    // included - IS the timing mechanism, not an incidental side effect.
    // Confirmed empirically: even juce::Thread::yield() (no minimum
    // delay) in the empty-buffer case measurably slowed playback.
    // Reverted to the original unconditional tight loop.
    void run() override {
        //setPriority(juce::Thread::Priority::low);
        while (!threadShouldExit()) {

            PlayingDevices = NoOfPlayingDevices.get();

            for (int i = 0; i < MyPlayingDevices; i++) {
                bool cie = false;
                switch (i) {
                case 0:  cie = ringBuffer0.remove(value);
                    B1F = cie; //store if buffer is empty or full
                    break;
                case 1:  cie = ringBuffer1.remove(value);
                    B2F = cie;
                    break;
                case 2:  cie = ringBuffer2.remove(value);
                    B3F = cie;
                    break;
                }

                if (cie) {
                    bool RS = false;
                    while (!RS){
                        RS = HardSID_WriteWithTimeout(i, cycles, value.SIDRegister, value.SIDData);
                        if (!RS) juce::Thread::sleep(20);
                    }
                }
                else { // buffer empty
                    bool RS = false;
                    while (!RS) {
                        RS = HardSID_WriteWithTimeout(i, cycles, 0x1e, 0); // do nothing
                        if (!RS) juce::Thread::sleep(20);
                     }
                }

            }
            if (PlayingDevices > MyPlayingDevices) {
                MyPlayingDevices = PlayingDevices;
            }
            if (PlayingDevices < MyPlayingDevices) { //We need to be sure the buffers are empty before decreasing the number of playing devices.
                if ((PlayingDevices == 2) && !B3F) MyPlayingDevices = PlayingDevices;
                if ((PlayingDevices == 1) && !B3F && !B2F) MyPlayingDevices = PlayingDevices;
                if ((PlayingDevices == 0) && !B3F && !B2F && !B1F) MyPlayingDevices = PlayingDevices;
            }
            if (!MyPlayingDevices) { // do nothing if player idle
                HardSID_WriteWithTimeout(0, 312 * 63 * 50, 0x1e, 0);// There is at least one device present,
                                                                    // otherwise the PlayThread would not have started
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
        return true;
    }

private:
    const int LOOP_TIME_OUT_MILLIS = 500;
    const int cycles = 8;
    SIDWriteSet value ;
    ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE>& ringBuffer0;
    ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE>& ringBuffer1;
    ThreadSafeRingBuffer<SIDWriteSet, MY_BUFFER_SIZE>& ringBuffer2;
    juce::Atomic<int>& NoOfPlayingDevices;
    int PlayingDevices = 0;
    int MyPlayingDevices = 0;
    bool B1F = false;
    bool B2F = false;
    bool B3F = false;
};
