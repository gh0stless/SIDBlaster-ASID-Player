/*
  ==============================================================================

    MainComponent.h
    Created: 22 Oct 2024 10:28:42am
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "Sid.h" 
#include "LedIndicator.h" 
#include "Mutex.h"

/* SID Register order for ASID */
static const uint8_t asid_sid_registers[] =
{
    /* register, bit */
    0x00, // 0
    0x01, // 1
    0x02, // 2
    0x03, // 3
    0x05, // 4
    0x06, // 5
    0x07, // 6
    0x08, // 7
    0x09, // 8
    0x0a, // 9
    0x0c, // 10
    0x0d, // 11
    0x0e, // 12
    0x0f, // 13
    0x10, // 14
    0x11, // 15
    0x13, // 16
    0x14, // 17
    0x15, // 18
    0x16, // 19
    0x17, // 20
    0x18, // 21
    0x04, // 22
    0x0b, // 23
    0x12, // 24
    0x04, // 25 <= secondary for reg 04
    0x0b, // 26 <= secondary for reg 11
    0x12, // 27 <= secondary for reg 18
};

class MainComponent :   public juce::Component,
                        public juce::ComboBox::Listener,
                        public juce::MidiInputCallback,
                        private juce::AsyncUpdater,
                        public juce::Timer
{
    
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;
    //==============================================================================
    void setErrorState(bool hasError); 
   
private:
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void handleAsyncUpdate() override;
    void comboBoxChanged(juce::ComboBox* comboBox) override; 
    void timerCallback() override; 
    void saveComboBoxSelection(); 
    void loadComboBoxSelection(); 
    void updateNoOfPlayingDevices(int newCount);
    
    juce::Atomic<bool> Msg1Mem{ false };
    juce::Atomic<bool> Msg2Mem{ false };
    juce::Atomic<bool> Msg3Mem{ false };

    juce::Atomic<bool> SID1isPlaying{ false };
    juce::Atomic<bool> SID2isPlaying{ false };
    juce::Atomic<bool> SID3isPlaying{ false };

    juce::ComboBox midiDeviceSelector;
    std::unique_ptr<juce::MidiInput> midiInput;
    juce::TextEditor outputTextBox;      
    juce::ApplicationProperties appProperties;
    
    Sid* sid;
    LedIndicator led; 
    
    juce::Image backgroundImage;
    
    juce::Time lastMidiDataTime0;
    juce::Time lastMidiDataTime1;
    juce::Time lastMidiDataTime2;
    
    juce::CriticalSection timeMutex;
    juce::CriticalSection midiMonitorLock;
    
    juce::Array<juce::MidiMessage> incomingMessages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
