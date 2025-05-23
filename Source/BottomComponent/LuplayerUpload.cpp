/*
  ==============================================================================

    LuplayerUpload.cpp
    Created: 21 May 2025 12:19:29pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LuplayerUpload.h"
#include <atomic>
#include <memory>
#include "../Settings/Settings.h"

//==============================================================================
LuplayerUpload::LuplayerUpload()
{
    fetchButton.onClick = [this] { showUuidPopup(); };
    addAndMakeVisible(fetchButton);

    downloadStatusLabel = std::make_unique<juce::Label>();
    downloadStatusLabel->setText("Go to upload.luplayer.org to upload sounds", juce::dontSendNotification);
    downloadStatusLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(downloadStatusLabel.get());
}

LuplayerUpload::~LuplayerUpload()
{
}

void LuplayerUpload::fetchButtonClicked()
{
    fetchJSON();
}

void LuplayerUpload::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
}

void LuplayerUpload::fetchJSON()
{
    filesSentToPlayer = false;
    juce::String uuid = uuidInput.getText().trim();

    juce::URL url("https://upload.luplayer.org/api/json/" + uuid);

    juce::StringPairArray responseHeaders;
    int statusCode = 0;
    std::unique_ptr<juce::InputStream> stream(
        url.createInputStream(
            false, nullptr, nullptr, {}, 2000, &responseHeaders, &statusCode
        )
    );

    if (stream != nullptr && statusCode == 200)
    {
        juce::String jsonText = stream->readEntireStreamAsString();
        std::vector<AudioFileInfo*> audioFiles = parseAudioFileList(jsonText);

        for (const auto& file : audioFiles)
        {
            DBG("File: " + file->name + ", Size: " + juce::String(file->size));
        }

        fetchAudioFiles(audioFiles);
    }
    else
    {
        juce::MessageManager::callAsync([uuid, statusCode]
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Error",
                    "\nCheck your internet connection or PIN number."
                );
            });

        DBG("Failed to fetch JSON for UUID: " + uuid + ", HTTP status: " + juce::String(statusCode));
    }
}

std::vector<AudioFileInfo*> LuplayerUpload::parseAudioFileList(const juce::String& jsonText)
{
    std::vector<AudioFileInfo*> audioFilePtrs;
    juce::var result = juce::JSON::parse(jsonText);

    if (result.isArray())
    {
        auto* jsonArray = result.getArray();
        for (auto& item : *jsonArray)
        {
            if (auto* obj = item.getDynamicObject())
            {
                auto* info = new AudioFileInfo();
                info->name = obj->getProperty("name").toString();
                info->size = (juce::int64)obj->getProperty("size");
                info->type = obj->getProperty("type").toString();
                info->extension = obj->getProperty("extension").toString();
                info->serverFile = obj->getProperty("serverFile").toString();
                audioFilePtrs.push_back(info);
            }
        }
    }
    else
    {
        DBG("JSON is not an array!");
    }
    return audioFilePtrs;
}

void LuplayerUpload::fetchAudioFiles(std::vector<AudioFileInfo*>& audioFilesInfoList)
{
    orderedFiles = audioFilesInfoList;
    filesRemaining = static_cast<int>(audioFilesInfoList.size());
    totalFilesToDownload = filesRemaining;

    // Show and update the label
    updateDownloadStatusLabel();

    //downloadStatusLabel->setVisible(filesRemaining > 0);

    {
        juce::ScopedLock lock(downloadedFilesLock);
        downloadedFiles.clear();
    }
    // Remove old progress bars and labels
    for (auto& pb : progressBars)
        removeChildComponent(pb.get());
    for (auto& lbl : progressLabels)
        removeChildComponent(lbl.get());
    for (auto& sz : progressSizeLabels)
        removeChildComponent(sz.get());
    progressBars.clear();
    progressLabels.clear();
    progressSizeLabels.clear();

    // Create new progress bars and labels
    for (size_t i = 0; i < audioFilesInfoList.size(); ++i)
    {
        // Name label
        auto* nameLabel = new juce::Label();
        nameLabel->setText(audioFilesInfoList[i]->name, juce::dontSendNotification);
        nameLabel->setJustificationType(juce::Justification::centredLeft);
        progressLabels.emplace_back(nameLabel);
        addAndMakeVisible(nameLabel);

        // Progress bar
        auto* bar = new juce::ProgressBar(audioFilesInfoList[i]->progress);
        bar->setColour(juce::ProgressBar::foregroundColourId, juce::Colours::orange);
        progressBars.emplace_back(bar);
        addAndMakeVisible(bar);

        // Size label
        auto* sizeLabel = new juce::Label();
        double fileSizeMB = audioFilesInfoList[i]->size / (1024.0 * 1024.0);
        juce::String fileSizeString = juce::String(fileSizeMB, 2) + " MB";
        sizeLabel->setText(fileSizeString, juce::dontSendNotification);
        sizeLabel->setJustificationType(juce::Justification::centredRight);
        progressSizeLabels.emplace_back(sizeLabel);
        addAndMakeVisible(sizeLabel);
    }

    resized(); // Layout progress bars and labels

    for (const auto& file : audioFilesInfoList)
    {
        downloadThreadPool.addJob(new DownloadJob(*this, file), true); // Pass pointer directly
    }
}

juce::File LuplayerUpload::fetchAudioFile(AudioFileInfo& fileInfo)
{
    const juce::String urlString = "https://upload.luplayer.org/api/uploads/" + fileInfo.serverFile;
    juce::URL url(urlString);

    std::unique_ptr<juce::InputStream> stream(url.createInputStream(false));

    if (stream != nullptr)
    {
        juce::String baseName = fileInfo.name;
        juce::String uniqueName = fileInfo.serverFile + "_" + baseName;


        DBG("Downloading: " + uniqueName);
#if RFBUILD
        juce::File destFile = juce::File(Settings::convertedSoundsPath).getChildFile(uniqueName);
#else
        juce::File destFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(uniqueName);

#endif


        if (destFile.existsAsFile())
            destFile.deleteFile();

        std::unique_ptr<juce::FileOutputStream> output(destFile.createOutputStream());

        if (output != nullptr)
        {
            constexpr int bufferSize = 8192;
            juce::HeapBlock<char> buffer(bufferSize);
            juce::int64 totalBytesRead = 0;
            juce::int64 totalSize = stream->getTotalLength();

            while (!stream->isExhausted())
            {
                const int bytesRead = stream->read(buffer, bufferSize);
                if (bytesRead <= 0)
                    break;

                output->write(buffer, bytesRead);
                totalBytesRead += bytesRead;

                fileInfo.progress = (totalSize > 0) ? static_cast<double>(totalBytesRead) / static_cast<double>(totalSize) : 0.0;

                juce::MessageManager::callAsync([this] { updateProgressBars(); });

                
            }

            
        }
        else
        {
            juce::MessageManager::callAsync([name = fileInfo.name] {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Error",
                    "Failed to download file: " + name
                );
            });
            DBG("Failed to write file: " + fileInfo.name);
            return juce::File();
        }
        output->flush();
        DBG("Download complete for: " + fileInfo.name);
        return destFile;
    }
    else
    {
        juce::MessageManager::callAsync([urlString] {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Error",
                "Failed to open URL: " + urlString
            );
        });
        DBG("Failed to open URL: " + urlString);
    }
    return juce::File();
}

juce::Array<AudioFileInfo> LuplayerUpload::getFilesToLoad()
{
	return downloadedFiles;
}



void LuplayerUpload::onAllDownloadsFinished()
{
    // Remove all progress bars and labels from the UI
    for (auto& pb : progressBars)
        removeChildComponent(pb.get());
    for (auto& lbl : progressLabels)
        removeChildComponent(lbl.get());
    for (auto& sz : progressSizeLabels)
        removeChildComponent(sz.get());
    progressBars.clear();
    progressLabels.clear();
    progressSizeLabels.clear();



    
    resized(); // Optionally update layout

    DBG("All downloads finished!");

    if (filesSentToPlayer == false)
    { 
        loadIntoPlayerBroadcaster->sendChangeMessage();
        filesSentToPlayer = true;
    }
}

void LuplayerUpload::updateProgressBars()
{
    // This will repaint the progress bars with the latest progress values
    for (auto& pb : progressBars)
    {
        pb->repaint();
    }
       
}

void LuplayerUpload::resized()
{
    fetchButton.setBounds(10, 10, 120, 30);
    downloadStatusLabel->setBounds(fetchButton.getRight() + 10, 10, 250, 30);


    int y = 50;
    int rowHeight = 24;
    int nameWidth = 220;
    int sizeWidth = 100;
    int spacing = 10;
    int x = 10;

    // Calculate the available width for the progress bar
    int barWidth = getWidth() - (x + nameWidth + spacing + sizeWidth + spacing + x);

    for (size_t i = 0; i < progressBars.size(); ++i)
    {
        int xPos = x;
        if (i < progressLabels.size())
            progressLabels[i]->setBounds(xPos, y, nameWidth, rowHeight);
        xPos += nameWidth + spacing;

        progressBars[i]->setBounds(xPos, y, barWidth, rowHeight);
        xPos += barWidth + spacing;

        if (i < progressSizeLabels.size())
            progressSizeLabels[i]->setBounds(xPos, y, sizeWidth, rowHeight);

        y += rowHeight + spacing;
    }
}

void LuplayerUpload::showUuidPopup()
{
    juce::AlertWindow aw("Enter PIN", "Go to upload.luplayer.org to upload files", juce::AlertWindow::QuestionIcon);
    aw.addTextEditor("pin", "", "PIN:", false);
    aw.addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    // Optionally set a default value
    aw.getTextEditor("pin")->setText(uuidInput.getText());
	aw.getTextEditor("pin")->setJustification(juce::Justification::centred);
	aw.getTextEditor("pin")->setInputRestrictions(6, "0123456789"); // Limit to 36 characters (UUID length)

    if (aw.runModalLoop() == 1)
    {
        juce::String uuid = aw.getTextEditor("pin")->getText().trim();
        if (!uuid.isEmpty())
        {
            uuidInput.setText(uuid, juce::dontSendNotification); // update the main input if you want
            fetchJSON();
        }
    }
}

void LuplayerUpload::removeProgressRowForFile(AudioFileInfo* fileInfo)
{
    // Find the index of the file in your progress list
    for (size_t i = 0; i < progressLabels.size(); ++i)
    {
        if (progressLabels[i]->getText() == fileInfo->name)
        {
            // Remove and delete the UI components
            if (i < progressBars.size())
            {
                removeChildComponent(progressBars[i].get());
                progressBars.erase(progressBars.begin() + i);
            }
            if (i < progressLabels.size())
            {
                removeChildComponent(progressLabels[i].get());
                progressLabels.erase(progressLabels.begin() + i);
            }
            if (i < progressSizeLabels.size())
            {
                removeChildComponent(progressSizeLabels[i].get());
                progressSizeLabels.erase(progressSizeLabels.begin() + i);
            }
            resized();
            break;
        }
    }

    filesRemaining = static_cast<int>(progressBars.size());
    updateDownloadStatusLabel();


    if (filesRemaining == 0)
        onAllDownloadsFinished();
}

void LuplayerUpload::updateDownloadStatusLabel()
{
    int downloaded = totalFilesToDownload - filesRemaining;
    if (filesRemaining > 0)
        downloadStatusLabel->setText("Downloading sounds... " + juce::String(downloaded) + " / " + juce::String(totalFilesToDownload), juce::dontSendNotification);
    else
        downloadStatusLabel->setText("Go to upload.luplayer.org to upload sounds", juce::dontSendNotification);
}