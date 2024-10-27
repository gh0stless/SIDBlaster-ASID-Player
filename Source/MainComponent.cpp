//==============================================================================
// SIDBlaster ASID Protocol Player
// by Andreas Schumm (gh0stless) 2024
// Version 0.1.2

#include "MainComponent.h"
#include "Sid.h" 

MainComponent::MainComponent()
{
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Hintergrund_png, BinaryData::Hintergrund_pngSize);
    
    addAndMakeVisible(led);
    led.setBlinkingRed(false); 
    led.setOn(true); // LED einschalten

    addAndMakeVisible(midiDeviceSelector);
    midiDeviceSelector.addListener(this);
    midiDeviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::lightgreen);

    juce::PropertiesFile::Options options;
    options.applicationName = "SIDBLaster ASID Player";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";

    appProperties.setStorageParameters(options);
    
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

    // TextEditor für Ausgaben konfigurieren
    addAndMakeVisible(outputTextBox);
    outputTextBox.setMultiLine(true);
    outputTextBox.setReadOnly(true);
    outputTextBox.applyColourToAllText(juce::Colours::lightgreen);
    outputTextBox.setScrollbarsShown(true);

    outputTextBox.insertTextAtCaret("SIDBlaster ASID Protocol Player 0.1.2 (alpha)\n");
    outputTextBox.insertTextAtCaret("by gh0stless 2024\n");
        
    sid = new Sid();
    outputTextBox.insertTextAtCaret("DLL Version: " +  juce::String(sid->GetDLLVersion()) + "\n");
    if (sid->error_state) {
        setErrorState(true);
        outputTextBox.insertTextAtCaret("No Sidblaster detected!\n");
    }
    else {
        sid->startPlayerThread();
        if (sid->Number_Of_Devices > 1) sid->Number_Of_Devices = 1; // *** Wir benutzen nur einen Sidblaster
        for (int i = 0; i < sid->Number_Of_Devices; i++) {
            sid->init(i);
            auto SIDTYPE = sid->GetSidType(i);
            outputTextBox.insertTextAtCaret(juce::String(i+1) + ": ");
            if (SIDTYPE == 0)  outputTextBox.insertTextAtCaret("Unknown SID Type detected\n");
            else if (SIDTYPE == 1)  outputTextBox.insertTextAtCaret("6581 SID detected\n");
            else if (SIDTYPE == 2)  outputTextBox.insertTextAtCaret("8580 SID detected\n");
        }
    }
}

MainComponent::~MainComponent()
{
    // MIDI Input stoppen, wenn die Komponente zerstört wird
    if (midiInput != nullptr)
    {
        midiInput->stop();
        midiInput = nullptr;
    }
    for (int i = 0; i < sid->Number_Of_Devices; i++) {
        sid->init(i);
        sid->stopPlayerThread();
    }
    delete sid;
    saveComboBoxSelection(); // Speichere die Auswahl beim Beenden der Anwendung  
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Überprüfen, ob das Bild geladen wurde
    if (backgroundImage.isValid())
    {
        // Zeichnet das Bild in voller Größe der Komponente
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        g.fillAll(juce::Colours::lightgrey); // Fallback, wenn das Bild nicht geladen wurde
    }

}

void MainComponent::resized()
{
    led.setBounds(getWidth() - 30, 10, 20, 20); // Setze die Position und Größe der LED
    midiDeviceSelector.setBounds(10, 40, getWidth() - 20, 20); // Setze die Position und Größe der ComboBox
    outputTextBox.setBounds(10, 70, getWidth() - 20, getHeight() - 80); // TextEditor für Ausgaben
}

//==============================================================================
void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (message.isSysEx())
    {
        
        lastMidiDataTime = juce::Time::getCurrentTime();
        const Uint8* data = message.getSysExData();
        int dataSize = message.getSysExDataSize();
       
        if ((dataSize > 3) && ((data[0]==45) && (data[1]==78 || data[1] == 80 || data[1] == 81))) {
            unsigned int reg = 0;
            for (uint8_t mask = 0; mask < 4; mask++) {  /* no more then 4 masks */
                for (uint8_t bit = 0; bit < 7; bit++) {  /* each packet has 7 bits ~ stoopid midi */
                    if (data[mask + 2] & (1 << bit)) {  /* 3 byte message, skip 3 each round and add the bit */
                        uint8_t register_value = data[reg + 10];  /* get the value to write from the buffer */
                        if (data[mask + 6] & (1 << bit)) {  /* if anything higher then 0 */
                            register_value |= 0x80;  /* the register_value needs its 8th MSB bit */
                        }
                        uint8_t address = asid_sid_registers[mask * 7 + bit];

                        if ((data[1] == 78) && (sid->Number_Of_Devices > 0)) {
                            if (!Msg1Mem) {
                                outputTextBox.insertTextAtCaret("ASID data recived, start playing...\n");
                                startTimer(500);
                                Msg1Mem = true;
                            }
                            sid->push_event(0, address, register_value);
                        }
                        if ((data[1] == 80) && (sid->Number_Of_Devices > 1)) {
                            if (!Msg2Mem) {
                                outputTextBox.insertTextAtCaret("2SID data recived, not supported\n");
                                
                                Msg2Mem = true;
                            }
                            //sid->push_event(1, address, register_value);
                        }
                        if ((data[1] == 81) && (sid->Number_Of_Devices > 2)) {
                            if (!Msg3Mem) {
                                outputTextBox.insertTextAtCaret("3SID data recived, not supported\n");
                                
                                Msg3Mem = true;
                            }
                            //sid->push_event(2, address, register_value);
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
}

// Callback-Methode für die ComboBox
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
    auto timeSinceLastMidi = currentTime - lastMidiDataTime;

    if (timeSinceLastMidi.inMilliseconds() >= 3000)  // Überprüfe, ob 3 Sekunden ohne MIDI-Daten vergangen sind
    {
        outputTextBox.insertTextAtCaret("no more ASID data, stop playing\n");
        stopTimer();
        Msg1Mem = false;
    }
}

// Methode zum Speichern der ComboBox-Auswahl
void MainComponent::saveComboBoxSelection()
{
    auto* propertiesFile = appProperties.getUserSettings();
    if (propertiesFile != nullptr) 
    {
        propertiesFile->setValue("midiDevice", midiDeviceSelector.getSelectedId());
        propertiesFile->saveIfNeeded(); // Speichere nur, wenn Änderungen gemacht wurden
    }
}

// Methode zum Laden der ComboBox-Auswahl
void MainComponent::loadComboBoxSelection()
{
    auto* propertiesFile = appProperties.getUserSettings();

    // Wenn die Datei nicht existiert oder leer ist, initialisiere sie
    if (propertiesFile == nullptr || !propertiesFile->containsKey("midiDevice"))
    {
        // Wähle das erste MIDI-Gerät aus der Liste als Standard aus, wenn vorhanden
        if (midiDeviceSelector.getNumItems() > 0)
        {
            midiDeviceSelector.setSelectedItemIndex(0);
        }
        saveComboBoxSelection(); // Speichere die Standardauswahl
    }
    else
    {
        // Lade die gespeicherte Auswahl
        int selectedDeviceId = propertiesFile->getIntValue("midiDevice", 0); // Standard-ID ist 0
        if (selectedDeviceId > 0)
        {
            midiDeviceSelector.setSelectedId(selectedDeviceId);
        }
    }
}




