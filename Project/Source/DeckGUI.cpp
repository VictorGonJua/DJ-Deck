/*
  ==============================================================================

    DeckGUI.cpp
    Created: 1 Sep 2022 9:42:23am
    Author:  Victor Gonzalez

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DeckGUI.h"

//==============================================================================
DeckGUI::DeckGUI(DJAudioPlayer* _player, 
    juce::AudioFormatManager& formatManagerToUse,
    juce::AudioThumbnailCache& cacheToUse)
    : player(_player), waveformDisplay(formatManagerToUse,cacheToUse)


{
    // loading stop and play icons
    auto pauseImage = juce::ImageCache::getFromMemory(BinaryData::pausebutton01_png, BinaryData::pausebutton01_pngSize);
    auto playImage = juce::ImageCache::getFromMemory(BinaryData::playbutton01_png, BinaryData::playbutton01_pngSize);

    // making visible all components of the deck
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(posSlider);
    addAndMakeVisible(trackName);
    addAndMakeVisible(loopButton);


    // listeners addition
    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);
    loopButton.addListener(this);

    // setting image attached to stop and play icons
    stopButton.setImages(true,true,true,pauseImage, 0.5f, juce::Colours::transparentBlack, pauseImage, 1.0f, juce::Colours::transparentBlack, pauseImage, 0.5f, juce::Colours::transparentBlack, 0.0f);
    playButton.setImages(true, true, true, playImage, 0.5f, juce::Colours::transparentBlack, playImage, 1.0f, juce::Colours::transparentBlack, playImage, 0.5f, juce::Colours::transparentBlack, 0.0f);

    // range setting for volume, speed and position sliders
    volSlider.setRange(0.0, 1.0);
    speedSlider.setRange(0.0, 10.0);
    posSlider.setRange(0.0, 1.0);

    // changing sliders stile to rotary
    volSlider.setSliderStyle(juce::Slider::Rotary);
    speedSlider.setSliderStyle(juce::Slider::Rotary);
    
    // adding text boxes to volume and speed sliders
    volSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100,25);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    
    // setting text to volume slider
    volSliderLabel.setText("Volume", juce::NotificationType::dontSendNotification);
    volSliderLabel.attachToComponent(&volSlider, false);

    // setting text to speed slider
    speedSliderLabel.setText("Speed", juce::NotificationType::dontSendNotification);
    speedSliderLabel.attachToComponent(&speedSlider, false);

    // display track name on top of the deck or "no track loaded"
    trackName.setReadOnly(true);
    trackName.setTextToShowWhenEmpty("No track loaded", juce::Colours::indianred);

    
    
    
    // add and make visible text labels
    addAndMakeVisible(speedSliderLabel);
    addAndMakeVisible(volSliderLabel);

    
    // playback slider set to transparent and overlayed above waveform
    posSlider.setSliderStyle(juce::Slider::LinearBar);
    posSlider.setTextBoxStyle(juce::Slider::NoTextBox,true,0,0);
    posSlider.setColour(juce::Slider::trackColourId,juce::Colours::transparentWhite);

    //Timer loops every 100ms, checks whether user has the "Repeat" button turned on or off and also updates the current position in the waveform display
    startTimer(100);

}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::orange);
    g.setFont (14.0f);
    g.drawText ("", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void DeckGUI::resized()
{

    double rowH = getHeight() / 7;

    playButton.setBounds((getWidth() / 6) * 1 + ((getWidth() / 6) * 1)/4, rowH, getWidth()/7, (rowH / 4) * 3);
    stopButton.setBounds((getWidth() / 6) * 4, rowH, getWidth() / 7, (rowH / 4) * 3);
    trackName.setBounds((getWidth() / 7) / 2, (rowH / 2) / 8, (getWidth() / 7) * 6, (rowH / 2) / 2);
    loopButton.setBounds((getWidth() / 6) * 4, rowH / 2, getWidth() / 8 * 2, rowH / 3);
    loadButton.setBounds((getWidth() / 6) * 1, rowH / 2, getWidth() / 8 * 2, rowH / 3);


    volSlider.setBounds(0, rowH*2, getWidth()/2, rowH*2);
    speedSlider.setBounds(getWidth()/2, rowH*2, getWidth()/2, rowH*2);

    volSliderLabel.setCentreRelative(0.4f, 0.44f);
    speedSliderLabel.setCentreRelative(0.925f, 0.44f);

   
    trackName.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    trackName.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    

    

    waveformDisplay.setBounds(0, rowH * 4, getWidth(), rowH*3);
    posSlider.setBounds(0, rowH * 4, getWidth(), rowH * 3);
 

}

void DeckGUI::buttonClicked(juce::Button* button) {
    if (button == &playButton) { //If button is clicked, start the audio file
        DBG("Play button is clicked");
        player->start();
    }
    if (button == &stopButton) { //If button is clicked, stops the audio file
        DBG("Stop button is clicked");
        player->stop();
    }
    if (button == &loadButton) { //If button is clicked, allows user to select file to be loaded into the deck
        juce::FileChooser chooser{ "Select a file..." };    
        if (chooser.browseForFileToOpen()) {
            player->loadURL(juce::URL{ chooser.getResult() });
            waveformDisplay.loadURL(juce::URL{ chooser.getResult() });
            DBG(juce::URL{ chooser.getResult() }.getFileName());

            trackName.setText(juce::URL{ chooser.getResult() }.getFileName(), juce::NotificationType::dontSendNotification);
        }
    }

}

void DeckGUI::sliderValueChanged(juce::Slider* slider) {
    if (slider == &volSlider) { //Sets audio file gain as vol slider value 
        player->setGain(slider->getValue());
    }
    if (slider == &speedSlider) {// Sets audio file speed as speed slider value
        player->setSpeed(slider->getValue());
    }
    if (slider == &posSlider) { //Sets audio file position as the position slider value
        player->setPositionRelative(slider->getValue());
    }

}

bool DeckGUI::isInterestedInFileDrag(const juce::StringArray& files) {
    DBG("DeckGUI::isInterestedInFileDrag");
    return true;
}
void DeckGUI::filesDropped(const juce::StringArray& files, int x, int y) {
    DBG("DeckGUI::filesDropped");
    if (files.size() == 1) {
        player->loadURL(juce::URL{ juce::File{files[0]} });
    }
}

void DeckGUI::timerCallback() { //Timer loops every 100ms, which checks if the "loop" button is on or off and also updates the wave form display current position
    if (std::to_string(loopButton.getToggleState()) == "1") { //If loop button is checked
        if (player->getPositionRelative() >= 1) {// and the position more than 1 meaning it is the end of the audio file
            player->setPositionRelative(0); // Sets position back to the start of the audio file
            player->start(); // Plays the audio file
        }
    }

    if (std::to_string(loopButton.getToggleState()) == "0") { //If loop button is not checked
        if (player->getPositionRelative() >= 1) { //and the position more than 1 meaning it is the end of the audio file
            player->setPositionRelative(0); //Sets position back to the start of the audio file
            player->stop(); // Stops the audio file
        }
    }
    waveformDisplay.setPositionRelative(player->getPositionRelative()); //Every 100ms, updates the waveform display to show current timestamp for the audio file
}


void DeckGUI::playlistToDeckGUI() { //function rea

        std::ifstream file("playlist.txt"); //Opens playlist.txt which contains the audio file URL which the user loads from the playlist into the deck
        std::string str;
        std::getline(file, str);
        std::string URL = "file:///" + str; //adds file:/// to the audio file URL, which converts the URL into a file
        DBG(URL);
        juce::URL audioURL{ URL };
        player->loadURL(audioURL); //Loads the file into the DJAudioPlayer
        waveformDisplay.loadURL(audioURL); //Loads the file and returns the waveform of the audio file
        DBG(audioURL.getFileName());

        mtrackName = audioURL.getFileName(); //Gets the file name from the URL
        
        trackName.setText(mtrackName, juce::NotificationType::dontSendNotification);
        

}
