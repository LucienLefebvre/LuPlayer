/*
  ==============================================================================

    RemoteInput.h
    Created: 3 Apr 2021 12:49:58pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <opus\opus.h>
#include <uvgrtp\lib.hh>
//#include "rtpsession.h"
//#include "rtpudpv4transmitter.h"
//#include "rtpipv4address.h"
//#include "rtpsessionparams.h"
//#include "rtperrors.h"
//#include "rtplibraryversion.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
//==============================================================================
/*
*/
#define FRAME_SIZE 480
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 256000
#define MAX_FRAME_SIZE 6*480
#define MAX_PACKET_SIZE (3*1276)

class RemoteInput  : public juce::Component,
    public juce::Thread
{
public:
    RemoteInput();
    ~RemoteInput() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    juce::AudioBuffer<float> bufferToSend;
    juce::AudioBuffer<float> decodedBuffer;
    juce::MemoryAudioSource decodedAudioSource;
    void RemoteInput::sendStream(juce::AudioBuffer<float>* soundBuffer);
    void RemoteInput::receiveStream(juce::AudioBuffer<float>* soundBuffer);
    void RemoteInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void RemoteInput::run();
    uvgrtp::context ctx;
    uvgrtp::session* sess;
    uvgrtp::media_stream* hevc;
    uvgrtp::media_stream* recv;

private:
    OpusEncoder* encoder;
    OpusDecoder* decoder;
    opus_int16 in[FRAME_SIZE * CHANNELS];
    //opus_int16 out[MAX_FRAME_SIZE * CHANNELS];
    int frame_size;
    int err;
    int err1;
    int nbBytes;
    unsigned char cbits[MAX_PACKET_SIZE];
    float out[MAX_FRAME_SIZE * CHANNELS];

    bool frame_received = false;
    uint32_t clock_rate = 90000 / 30;
    uint32_t timestamp = 0;

    //RTPSession session;
    //uint16_t portbase, destport;
    //uint32_t destip;
    ////RTPIPv4Address addr;
    //std::string ipstr;
    //int status, i, num;
    //RTPUDPv4TransmissionParams transparams;
    //RTPSessionParams sessparams;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemoteInput)
};
