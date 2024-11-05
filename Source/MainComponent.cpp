//==============================================================================
// SIDBlaster ASID Protocol Player
// by Andreas Schumm (gh0stless) 2024
// Version 0.9.7 beta
//
// ASID decoder routine was taken from the USBSID Piko project:
// https://github.com/LouDnl/USBSID-Pico
// 
// Hardsid library by Stein Pedersen & Ken Händel
// Thanks to Wilfred Bos for his tips and tricks

#include "MainComponent.h"

MainComponent::MainComponent()
{
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Hintergrund_png, BinaryData::Hintergrund_pngSize);
    
    juce::Component::addAndMakeVisible(led);
    led.setBlinkingRed(false); 
    led.setOn(false); // LED ausschalten

    juce::Component::addAndMakeVisible(midiDeviceSelector);
    midiDeviceSelector.addListener(this);
    midiDeviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::lightgreen);

    juce::PropertiesFile::Options options;
    options.applicationName = "SIDBlaster ASID Player";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    appProperties.setStorageParameters(options);
    
    // TextEditor für Ausgaben konfigurieren
    juce::Component::addAndMakeVisible(outputTextBox);
    outputTextBox.setMultiLine(true);
    outputTextBox.setReadOnly(true);
    outputTextBox.applyColourToAllText(juce::Colours::lightgreen);
    outputTextBox.setScrollbarsShown(true);

    outputTextBox.insertTextAtCaret("SIDBlaster ASID Protocol Player 0.9.7 (beta)\n");
    outputTextBox.insertTextAtCaret("by gh0stless 2024\n");

    // Füge alle verfügbaren MIDI-Geräte zur ComboBox hinzu
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& device : midiInputs)
    {
        midiDeviceSelector.addItem(device.name, device.identifier.hashCode());
    }

    // Lade die gespeicherte Auswahl
    loadComboBoxSelection();
    
    // MIDI Input initialisieren
    if (midiInputs.size() > 0)
    {
        midiInput = juce::MidiInput::openDevice(midiInputs[0].identifier, this);
        if (midiInput != nullptr)
        {
            midiInput->start();
        }
    }
            
    sid = new Sid();
    outputTextBox.insertTextAtCaret("DLL Version: " +  juce::String(sid->GetDLLVersion()) + "\n");
    if (sid->error_state) {
        setErrorState(true);
        outputTextBox.insertTextAtCaret("No Sidblaster detected!\n");
    }
    else {
        
        if (sid->Number_Of_Devices > 3) sid->Number_Of_Devices = 3; // *** Wir benutzen nur max. drei Sidblaster
        
        sid->startPlayerThread();
        //juce::Thread::sleep(100);
        while (!sid->isPlayerThreadRuning()) { ; }
        updateNoOfPlayingDevices(sid->Number_Of_Devices);
        juce::Thread::sleep(100);
        for (int i = 0; i < sid->Number_Of_Devices; i++) {
            sid->init(i);
            auto SIDTYPE = sid->GetSidType(i);
            outputTextBox.insertTextAtCaret(juce::String(i+1) + ": ");
            if (SIDTYPE == 0)  outputTextBox.insertTextAtCaret("Unknown SID Type detected\n");
            else if (SIDTYPE == 1)  outputTextBox.insertTextAtCaret("6581 SID detected\n");
            else if (SIDTYPE == 2)  outputTextBox.insertTextAtCaret("8580 SID detected\n");
        }
        startTimer(500);
        juce::Thread::sleep(100);
        updateNoOfPlayingDevices(0);
    }
 }

MainComponent::~MainComponent(){

    saveComboBoxSelection(); // store selected midi device

    if (midiInput != nullptr)
    {
        midiInput->stop();
        midiInput = nullptr;
    }

    updateNoOfPlayingDevices(sid->Number_Of_Devices);
    for (int i = 0; i < sid->Number_Of_Devices; i++) {
        sid->init(i);
    }

    stopTimer();
    updateNoOfPlayingDevices(0);
    sid->stopPlayerThread();
    delete sid;
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Component::getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Überprüfen, ob das Bild geladen wurde
    if (backgroundImage.isValid())
    {
        // Zeichnet das Bild in voller Größe der Komponente
        g.drawImage(backgroundImage, juce::Component::getLocalBounds().toFloat());
    }
    else
    {
        g.fillAll(juce::Colours::lightgrey); // Fallback, wenn das Bild nicht geladen wurde
    }

}

void MainComponent::resized()
{
    led.setBounds(juce::Component::getWidth() - 30, 10, 20, 20); // Setze die Position und Größe der LED
    midiDeviceSelector.setBounds(10, 40, juce::Component::getWidth() - 20, 20); // Setze die Position und Größe der ComboBox
    outputTextBox.setBounds(10, 70, juce::Component::getWidth() - 20, juce::Component::getHeight() - 80); // TextEditor für Ausgaben
}
//==============================================================================
void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (message.isSysEx())
    {
        const juce::ScopedLock sl(midiMonitorLock);
        incomingMessages.add(message);
        triggerAsyncUpdate(); // Asynchrones Update starten
    }
}

void MainComponent::handleAsyncUpdate() {

    // This is called on the message loop
    juce::Array<juce::MidiMessage> messages;

    {
        const juce::ScopedLock sl(midiMonitorLock);
        messages.swapWith(incomingMessages);
    }
        
    // Durchlaufe alle empfangenen MIDI-Nachrichten
    for (const auto& message : messages){

        const Uint8* data = message.getSysExData();
        int dataSize = message.getSysExDataSize();

        //decodiere ASID MIDI Daten
        if ((dataSize > 3) && ((data[0] == 45) && (data[1] == 78 || data[1] == 80 || data[1] == 81))) {
            unsigned int reg = 0;
            for (uint8_t mask = 0; mask < 4; mask++) {  /* no more then 4 masks */
                for (uint8_t bit = 0; bit < 7; bit++) {  /* each packet has 7 bits ~ stoopid midi */
                    if (data[mask + 2] & (1 << bit)) {  /* 3 byte message, skip 3 each round and add the bit */
                        uint8_t register_value = data[reg + 10];  /* get the value to write from the buffer */
                        if (data[mask + 6] & (1 << bit)) {  /* if anything higher then 0 */
                            register_value |= 0x80;  /* the register_value needs its 8th MSB bit */
                        }
                        uint8_t address = asid_sid_registers[mask * 7 + bit];

                        //Verarbeite Daten (Ausgabe auf SIDBlaster)
                        if ((data[1] == 78)) {
                            {   
                                const juce::ScopedLock lock(timeMutex);
                                lastMidiDataTime0 = juce::Time::getCurrentTime();
                            }
                            if (sid->Number_Of_Devices > 0) {
                                if (!SID1isPlaying.get()) {
                                    if (!Msg1Mem.get()) {
                                        outputTextBox.insertTextAtCaret("ASID data recived, start playing SID1...\n");
                                        Msg1Mem.set(true);
                                        led.setOn(true);
                                    }
                                    if (!SID1isPlaying.get() && (!(SID2isPlaying.get() || SID3isPlaying.get()))) {
                                        updateNoOfPlayingDevices(1);
                                        SID1isPlaying.set(true);
                                    }
                                }
                                if (sid->Number_Of_Devices > 0) sid->push_event(0, address, register_value);
                            }
                            else {
                                if (!Msg1Mem.get()) {
                                    outputTextBox.insertTextAtCaret("You need more SIDBlasters for ASID\n");
                                    Msg1Mem.set(true);
                                }
                            }
                        }
                        if (data[1] == 80) {
                            {
                                const juce::ScopedLock lock(timeMutex);
                                lastMidiDataTime1 = juce::Time::getCurrentTime();
                            }
                            if (sid->Number_Of_Devices > 1) {
                                if (!SID2isPlaying.get()) {
                                    if (!Msg2Mem.get()) {
                                        outputTextBox.insertTextAtCaret("2SID data recived, start playing SID2...\n");
                                        Msg2Mem.set(true);
                                        led.setOn(true);
                                    }
                                    if (!SID3isPlaying.get()) {
                                        updateNoOfPlayingDevices(2);
                                        SID2isPlaying.set(true);
                                    }
                                }
                                sid->push_event(1, address, register_value);
                            }
                            else {
                                if (!Msg2Mem.get()) {
                                    outputTextBox.insertTextAtCaret("You need more SIDBlasters for 2SID\n");
                                    Msg2Mem.set(true);
                                }
                            }
                            
                        }

                        if (data[1] == 81) {
                            {
                                const juce::ScopedLock lock(timeMutex);
                                lastMidiDataTime2 = juce::Time::getCurrentTime();
                            }
                            if (sid->Number_Of_Devices > 2) {
                                if (!SID3isPlaying.get()) {
                                    if (!Msg3Mem.get()) {
                                        outputTextBox.insertTextAtCaret("3SID data recived, start playing SID3...\n");
                                        Msg3Mem.set(true);
                                        led.setOn(true);
                                    }
                                    if (!SID3isPlaying.get()) {
                                        updateNoOfPlayingDevices(3);
                                        SID3isPlaying.set(true);
                                    }
                                }
                                sid->push_event(2, address, register_value);
                            }
                            else {
                                if (!Msg3Mem.get()) {
                                    outputTextBox.insertTextAtCaret("You need more SIDBlasters for 3SID\n");
                                    Msg3Mem.set(true);
                                }
                            }
                        }
                        reg++;
                    }
                }
            }
        }
        else if (data[0] == 45 && data[1] == 76) {
            outputTextBox.insertTextAtCaret("Start\n");
        }
        else if (data[0] == 45 && data[1] == 77) {
            outputTextBox.insertTextAtCaret("Stop\n");
        }
        else if (data[0] == 45 && data[1] == 79) {//Display Data

            for (int i = 2; i < dataSize; ++i)
            {
                outputTextBox.insertTextAtCaret(juce::String::charToString(data[i]) + " ");
            }
            outputTextBox.insertTextAtCaret("\n");
        }
    }
    incomingMessages.clear(); // Nachrichtencache leeren
}
//==============================================================================
void MainComponent::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &midiDeviceSelector)
    {
        auto selectedDeviceName = midiDeviceSelector.getText();
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        for (const auto& device : midiInputs)
        {
            if (device.name == selectedDeviceName)
            {
                if (midiInput != nullptr)
                {
                    midiInput->stop();
                    midiInput = nullptr;
                }

                if (auto newMidiInput = juce::MidiInput::openDevice(device.identifier, this))
                {
                    midiInput = std::move(newMidiInput);
                    midiInput->start();
                }
                break;
            }
        }
    }
}

void MainComponent::setErrorState(bool hasError)
{
    if (hasError)
    {
        led.setBlinkingRed(true); // LED blinken lassen
    }
    else
    {
        led.setBlinkingRed(false); // LED konstant
        led.setOn(true); // LED einschalten
    }
}

void MainComponent::timerCallback()
{
    if (led.isBlinkingState())
    {
        auto HV = led.isOnState();
        HV = !HV;
        led.setOn(HV); // LED-Status umschalten

    }
    // MIDI-Daten-Timeout-Logik
    auto currentTime = juce::Time::getCurrentTime();
    
    // Geschützten Zugriff auf die letzten MIDI-Zeiten
    juce::RelativeTime timeSinceLastMidi0, timeSinceLastMidi1, timeSinceLastMidi2;
    {
        const juce::ScopedLock lock(timeMutex);
        timeSinceLastMidi0 = currentTime - lastMidiDataTime0;
        timeSinceLastMidi1 = currentTime - lastMidiDataTime1;
        timeSinceLastMidi2 = currentTime - lastMidiDataTime2;
    }


    if (timeSinceLastMidi0.inMilliseconds() >= 3000)  // Überprüfe, ob 3 Sekunden ohne 1SID-Daten vergangen sind
    {
        if (Msg1Mem.get()) {
            outputTextBox.insertTextAtCaret("no more ASID data, stop playing SID1\n");
            Msg1Mem.set(false);
            if (!SID2isPlaying.get() && !SID3isPlaying.get()) updateNoOfPlayingDevices(0);
            SID1isPlaying.set(false);
            if ((!SID1isPlaying.get() && !SID2isPlaying.get() && !SID3isPlaying.get()) && led.isOnState()) led.setOn(false);
        }
        else if (Msg1Mem.get() && !SID1isPlaying.get()) {
            outputTextBox.insertTextAtCaret("no more ASID data\n");
            Msg1Mem.set(false);
        }
    }
    if (timeSinceLastMidi1.inMilliseconds() >= 3000)  // Überprüfe, ob 3 Sekunden ohne 2SID-Daten vergangen sind
    {
        if (Msg2Mem.get() && SID2isPlaying.get()) {
            outputTextBox.insertTextAtCaret("no more 2SID data, stop playing SID2\n");
            Msg2Mem.set(false);
            if (SID1isPlaying.get() && !SID3isPlaying.get()) updateNoOfPlayingDevices(1);
            if (!SID1isPlaying.get() && !SID3isPlaying.get()) updateNoOfPlayingDevices(0);
            SID2isPlaying.set(false);
            if ((!SID1isPlaying.get() && !SID2isPlaying.get() && !SID3isPlaying.get()) && led.isOnState()) led.setOn(false);
        }
        else if (Msg2Mem.get() && !SID2isPlaying.get()) {
            outputTextBox.insertTextAtCaret("no more 2SID data\n");
            Msg2Mem.set(false);
        }
    }
    if (timeSinceLastMidi2.inMilliseconds() >= 3000)  // Überprüfe, ob 3 Sekunden ohne 3SID-Daten vergangen sind
    {
        if (Msg3Mem.get() && SID3isPlaying.get()) {
            outputTextBox.insertTextAtCaret("no more 3SID data, stop playing SID3\n");
            Msg3Mem.set(false);
            if (SID1isPlaying.get() && SID2isPlaying.get()) updateNoOfPlayingDevices(2);
            if (SID1isPlaying.get() && !SID2isPlaying.get()) updateNoOfPlayingDevices(1);
            if (!SID1isPlaying.get() && !SID2isPlaying.get()) updateNoOfPlayingDevices(0);
            SID3isPlaying.set(false);
            if ((!SID1isPlaying.get() && !SID2isPlaying.get() && !SID3isPlaying.get()) && led.isOnState()) led.setOn(false);
        }
        else if (Msg3Mem.get() && !SID3isPlaying.get()) {
            outputTextBox.insertTextAtCaret("no more 3SID data\n");
            Msg3Mem.set(false);
        }
    }
}

void MainComponent::saveComboBoxSelection(){
    auto* propertiesFile = appProperties.getUserSettings();
    if (propertiesFile != nullptr) {
        propertiesFile->setValue("midiDevice", midiDeviceSelector.getSelectedId());
        if (midiDeviceSelector.getSelectedId() != 0) {
            propertiesFile->saveIfNeeded();
        }
    }
}

void MainComponent::loadComboBoxSelection(){
    auto* propertiesFile = appProperties.getUserSettings();

    // Wenn die Datei nicht existiert oder leer ist, initialisiere sie
    if (propertiesFile == nullptr || !propertiesFile->containsKey("midiDevice")){
        // Wähle das erste MIDI-Gerät aus der Liste als Standard aus, wenn vorhanden
        if (midiDeviceSelector.getNumItems() > 0){
            midiDeviceSelector.setSelectedItemIndex(0);
        }
        //saveComboBoxSelection(); // Speichere die Standardauswahl
    }
    else{
        // Lade die gespeicherte Auswahl
        int selectedDeviceId = propertiesFile->getIntValue("midiDevice", 0); // Standard-ID ist 0
        if (selectedDeviceId > 0){
            midiDeviceSelector.setSelectedId(selectedDeviceId);
        }
    }
}

void MainComponent::updateNoOfPlayingDevices(int newCount)
{
    No_Of_Playing_Devices.set(newCount);
}

