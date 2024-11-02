/*
  ==============================================================================

    Mutex.h
    Created: 1 Nov 2024 6:13:08pm
    Author:  andre

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>


//extern juce::CriticalSection noOfPlayingDevicesMutex;
//extern int No_Of_Playing_Devices;
extern juce::Atomic<int> No_Of_Playing_Devices;