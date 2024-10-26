#pragma once

#include <JuceHeader.h>
#include "Sid.h" 
#include "LedIndicator.h" 

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
                        public juce::Timer
{
    
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void setErrorState(bool hasError); 
    bool Msg1Mem = false;
    bool Msg2Mem = false;
    bool Msg3Mem = false;
private:
    void comboBoxChanged(juce::ComboBox* comboBox) override; // Callback-Methode für die ComboBox
    void timerCallback() override; // Timer-Callback deklarieren
    Uint8 number_of_bits(Uint8 value);

    void saveComboBoxSelection(); // Methode zum Speichern der Auswahl
    void loadComboBoxSelection(); // Methode zum Laden der Auswahl

    juce::ComboBox midiDeviceSelector; // ComboBox für MIDI-Geräte
    std::unique_ptr<juce::MidiInput> midiInput; // MidiInput-Objekt
    juce::TextEditor outputTextBox;      

    juce::ApplicationProperties appProperties;
    
    Sid* sid;
    int blinkTimer = 0;
    LedIndicator led; 
    juce::Image backgroundImage;

    int noMidiDataTimer = 0; // Timer für die MIDI-Datenüberwachung
    juce::Time lastMidiDataTime;  // Speichert den Zeitpunkt des letzten MIDI-Daten-Eingangs

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
