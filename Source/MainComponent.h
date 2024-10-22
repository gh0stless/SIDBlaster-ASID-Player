#pragma once

#include <JuceHeader.h>
#include "Sid.h" 
#include "LedIndicator.h" 

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
    
private:
    void comboBoxChanged(juce::ComboBox* comboBox) override; // Callback-Methode für die ComboBox
    void timerCallback() override; // Timer-Callback deklarieren
    Uint8 MainComponent::number_of_bits(Uint8 value);

    void saveComboBoxSelection(); // Methode zum Speichern der Auswahl
    void loadComboBoxSelection(); // Methode zum Laden der Auswahl

    juce::ComboBox midiDeviceSelector; // ComboBox für MIDI-Geräte
    std::unique_ptr<juce::MidiInput> midiInput; // MidiInput-Objekt
    juce::TextEditor outputTextBox;       // Textbox für Ausgaben

    juce::ApplicationProperties appProperties;
    
    Sid* sid;
    int blinkTimer = 0;
    LedIndicator led; // LED-Komponente
    juce::Image backgroundImage;

    bool Msg1Mem = false;
    bool Msg2Mem = false;
    bool Msg3Mem = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
