/*
  ==============================================================================

    Mutex.cpp
    Created: 1 Nov 2024 6:12:54pm
    Author:  andre

  ==============================================================================
*/

#include "Mutex.h"

//juce::CriticalSection noOfPlayingDevicesMutex;
//int No_Of_Playing_Devices = 0;
juce::Atomic<int> No_Of_Playing_Devices{ 0 };