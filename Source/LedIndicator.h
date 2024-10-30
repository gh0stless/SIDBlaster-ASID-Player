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

    // �berschreibt die paint-Methode, um die LED zu zeichnen
    void paint(juce::Graphics& g) override
    {
        // W�hle die entsprechende Farbe basierend auf dem Blinkzustand
        g.setColour(isOn ? blinkColour : juce::Colours::transparentBlack);
        g.fillEllipse(getLocalBounds().toFloat());
    }

    // Timer-Callback f�r das Blinken
    void timerCallback() override
    {
        setOn(!isOn); // Zustand der LED umschalten
    }

    // Getter f�r den Blinkzustand
    bool isBlinkingState() const { return isBlinking; }

    // Getter f�r den Zustand der LED
    bool isOnState() const { return isOn; }

private:
    bool isOn = false;         // Status der LED
    bool isBlinking = false;   // Blinkzustand der LED
    juce::Colour blinkColour;  // Farbe der blinkenden LED (rot oder gr�n)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedIndicator)
};
