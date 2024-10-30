/*
  ==============================================================================

    LedIndicator.h
    Created: 22 Oct 2024 10:28:42am
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

class LedIndicator : public juce::Component, public juce::Timer
{
public:
    LedIndicator() {}
    ~LedIndicator() override {}

    // Setzt den Zustand der LED und aktualisiert die Anzeige
    void setOn(bool state)
    {
        isOn = state;
        repaint();
    }

    // Setzt den Blinkzustand der LED
    void setBlinkingRed(bool state)
    {
        
        isBlinking = state;
        blinkColour = juce::Colours::red;
        if (state)
            startTimer(500); // Timer starten
        else
            stopTimer(); // Timer stoppen
    }

    // Setzt den Blinkzustand der LED
    void setBlinkingGreen(bool state)
    {
        isBlinking = state;
        blinkColour = juce::Colours::green;
        if (state)

            startTimer(500); // Timer starten
        else
            stopTimer(); // Timer stoppen
    }

    // Überschreibt die paint-Methode, um die LED zu zeichnen
    void paint(juce::Graphics& g) override
    {
        // Wähle die entsprechende Farbe basierend auf dem Blinkzustand
        g.setColour(isOn ? blinkColour : juce::Colours::transparentBlack);
        g.fillEllipse(getLocalBounds().toFloat());
    }

    // Timer-Callback für das Blinken
    void timerCallback() override
    {
        setOn(!isOn); // Zustand der LED umschalten
    }

    // Getter für den Blinkzustand
    bool isBlinkingState() const { return isBlinking; }

    // Getter für den Zustand der LED
    bool isOnState() const { return isOn; }

private:
    bool isOn = false;         // Status der LED
    bool isBlinking = false;   // Blinkzustand der LED
    juce::Colour blinkColour;  // Farbe der blinkenden LED (rot oder grün)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedIndicator)
};
