/*
  ==============================================================================

    LuplayerUpload.h
    Created: 21 May 2025 12:19:29pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FocusAwareTextEditor.h"
//==============================================================================
/*
*/
struct AudioFileInfo
{
    juce::String name;
    juce::int64 size;
    juce::String type;
    juce::String extension;
    juce::String serverFile;
    double progress = 0.0f;
    bool loadedIntoPlayer = false;
    juce::File file;
};

class LuplayerUpload  : public juce::Component
{
    class DownloadJob : public juce::ThreadPoolJob
    {
    public:
        DownloadJob(LuplayerUpload& owner, AudioFileInfo* fileInfo)
            : juce::ThreadPoolJob("DownloadJob"), owner(owner), fileInfo(fileInfo) {}

        JobStatus runJob() override
        {
            // Download the file
            fileInfo->file = owner.fetchAudioFile(*fileInfo);
            
            // Update downloadedFiles array with proper locking
            {
                juce::ScopedLock lock(owner.downloadedFilesLock);
                // Find the index in orderedFiles
                auto it = std::find(owner.orderedFiles.begin(), owner.orderedFiles.end(), fileInfo);
                if (it != owner.orderedFiles.end())
                {
                    int idx = static_cast<int>(std::distance(owner.orderedFiles.begin(), it));
                    // Ensure the array is large enough
                    if (owner.downloadedFiles.size() <= idx)
                        owner.downloadedFiles.resize(idx + 1);
                    owner.downloadedFiles.set(idx, *fileInfo);
                }
            }
            
            // Capture owner directly instead of 'this'
            juce::MessageManager::callAsync([&owner = owner, fileInfoPtr = fileInfo]() { 
                owner.removeProgressRowForFile(fileInfoPtr); 
            });
            
            return jobHasFinished;
        }

    private:
        LuplayerUpload& owner;
        AudioFileInfo* fileInfo; // Pointer instead of copy
    };

public:
    LuplayerUpload();
    ~LuplayerUpload() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Array<AudioFileInfo> getFilesToLoad();
	juce::Array<AudioFileInfo> downloadedFiles;

    bool loadIntoPlayer = true;
    bool loadIntoPool = true;

    std::unique_ptr<juce::ChangeBroadcaster> loadIntoPlayerBroadcaster = std::make_unique<juce::ChangeBroadcaster>();

    void updateProgressBars();

    void fetchButtonClicked();
    void showUuidPopup();

    void removeProgressRowForFile(AudioFileInfo* fileInfo);

    void updateDownloadStatusLabel();

private:
    bool filesSentToPlayer = false;
    std::unique_ptr<juce::Viewport> progressViewport;
    std::unique_ptr<juce::Component> contentComponent;
    std::vector<AudioFileInfo*> orderedFiles;

    juce::TextButton fetchButton{ "Import" };
    void fetchJSON();
    std::vector<AudioFileInfo*> parseAudioFileList(const juce::String& jsonText);

    void fetchAudioFiles(std::vector<AudioFileInfo*>& audioFilesInfoList);
    juce::File fetchAudioFile(AudioFileInfo& fileInfo);

    juce::ThreadPool downloadThreadPool{ 4 }; // 4 threads, adjust as needed
    std::atomic<int> filesRemaining{ 0 };
    void onAllDownloadsFinished();
    juce::CriticalSection downloadedFilesLock;

    std::vector<std::unique_ptr<juce::ProgressBar>> progressBars;
    std::vector<std::unique_ptr<juce::Label>> progressLabels;
    std::vector<std::unique_ptr<juce::Label>> progressSizeLabels;
    juce::Component* previousFocusComponent = nullptr;

    std::unique_ptr<juce::Label> downloadStatusLabel;
    int totalFilesToDownload = 0;

    // Replace the juce::TextEditor with this custom class
    FocusAwareTextEditor uuidInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LuplayerUpload)
};
