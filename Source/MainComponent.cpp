//==============================================================================
// SIDBlaster ASID Protocol Player
// by Andreas Schumm (gh0stless) 2024
// Version 0.1

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

    outputTextBox.insertTextAtCaret("SIDBlaster ASID Protocol Player 0.1\n");
    outputTextBox.insertTextAtCaret("by gh0stless 2024\n");
    outputTextBox.insertTextAtCaret("DLL Version: " +  juce::String(sid->GetDLLVersion()) + "\n");
    
    sid = new Sid();
    if (sid->error_state) {
        setErrorState(true);
        outputTextBox.insertTextAtCaret("No Sidblaster detected!\n");
    }
    else {
        for (int i = 0; i < sid->Number_Of_Devices; i++) {
            sid->init(i);
            sid->startConsumer(i);
            auto SIDTYPE = sid->GetSidType(i);
            outputTextBox.insertTextAtCaret(juce::String(i+1) + ": ");
            if (SIDTYPE == 0)  outputTextBox.insertTextAtCaret("Unknown SID Type detected\n");
            else if (SIDTYPE == 1)  outputTextBox.insertTextAtCaret("6581 SID detected\n");
            else if (SIDTYPE == 2)  outputTextBox.insertTextAtCaret("8580 SID detected\n");
        }
        outputTextBox.insertTextAtCaret("READY\n.\n");
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
        sid->stopConsumer(i);
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
        const Uint8* data = message.getSysExData();
        int dataSize = message.getSysExDataSize();
        
        Uint8 idtable[28] = { 0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 4, 11, 18, 4, 11, 18 };
        Uint8 dataP = 10;
        Uint8 sidRegister = 0;
        Uint8 sidData = 0;
        Uint8 LSBdata = 0;
        Uint8 NumberOfDataBytes = 0;

        if ((dataSize > 3) && ((data[0]==45) && (data[1]==78 || data[1] == 80 || data[1] == 81))) {
            for (int i = 0; i <= 3; i++) {
                NumberOfDataBytes = NumberOfDataBytes + number_of_bits(data[i + 2]);
            }
            if ((NumberOfDataBytes + 8 + 2) == dataSize) {
                for (int i = 0; i <= 3; i++) {
                    Uint8 count = 0;
                    for (Uint8 mask = 0x01; mask != 0x80; mask <<= 1) {
                        if (data[i + 2] & mask) {
                            //make a sid write
                            sidRegister = idtable[i * 7 + count];
                            LSBdata = data[dataP];
                            dataP++;
                            if (data[i + 6] & mask) {
                                sidData = LSBdata + 127;
                            }
                            else {
                                sidData = LSBdata;
                            }
                            if ((data[1] == 78) && (sid->Number_Of_Devices > 0)) {
                                if (!Msg1Mem) { outputTextBox.insertTextAtCaret("ASID Data recived, now playing...\n"); Msg1Mem = true; }
                                sid->push_event(0, sidRegister, sidData);
                            }
                            if ((data[1] == 80) && (sid->Number_Of_Devices > 1)) {
                                if (!Msg2Mem) { outputTextBox.insertTextAtCaret("2SID Data recived\n"); Msg2Mem = true; }
                                sid->push_event(1, sidRegister, sidData);
                            }
                            if ((data[1] == 81) && (sid->Number_Of_Devices > 2)) {
                                if (!Msg3Mem) { outputTextBox.insertTextAtCaret("3SID Data recived\n"); Msg3Mem = true; }
                                sid->push_event(2, sidRegister, sidData);
                            }
                        }
                        else {
                        }
                        count++;
                    }
                }
                dataP = 10;
            }
        }
        
        else if (data[0] == 45 && data[1] == 76) {
            outputTextBox.insertTextAtCaret("Start\n");
        }
        else if (data[0] == 45 && data[1] == 77) {
            outputTextBox.insertTextAtCaret("Stop\n");
        }
        else if (data[0] == 45 && data[1] == 79) {
            
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
}

Uint8 MainComponent::number_of_bits(Uint8 value) {
    Uint8 ones = 0;
    ones += value & 0x01;
    ones += (value & 0x02) >> 1;
    ones += (value & 0x04) >> 2;
    ones += (value & 0x08) >> 3;
    ones += (value & 0x10) >> 4;
    ones += (value & 0x20) >> 5;
    ones += (value & 0x40) >> 6;
    ones += (value & 0x80) >> 7;
    return ones;
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




