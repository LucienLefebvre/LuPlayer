/*
  ==============================================================================

    RemoteInput.cpp
    Created: 3 Apr 2021 12:49:58pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RemoteInput.h"

//==============================================================================
RemoteInput::RemoteInput() : bufferToSend(1, FRAME_SIZE), decodedBuffer(1, FRAME_SIZE), juce::Thread("thread"), decodedAudioSource(decodedBuffer, false, false)
{
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err1);
    err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));


    //UVGRTP
    sess = ctx.create_session("192.168.0.11");
    hevc = sess->create_stream(8889, 8888, RTP_FORMAT_OPUS, RCE_RTCP);
    hevc->get_rtcp()->set_ts_info(uvgrtp::clock::ntp::now(), clock_rate, timestamp);
    recv = sess->create_stream(8888, 8889, RTP_FORMAT_OPUS, RCE_RTCP);

    //sessparams.SetOwnTimestampUnit(1.0 / 48000.0);
    //sessparams.SetAcceptOwnPackets(true);
    //transparams.SetPortbase(8000);
    //int status = session.Create(sessparams, &transparams);
    //if (status < 0)
    //{
    //    DBG(RTPGetErrorString(status));

    //}
    //destip = ntohl(inet_addr("192.168.0.11"));
    ////uint8_t localip[] = { 192,168,0,11 };
    //RTPIPv4Address addr(destip, 9000);
    //session.AddDestination(addr);
    //if (status < 0)
    //{
    //    DBG(RTPGetErrorString(status));

    //}
    //session.SetDefaultPayloadType(96);
    //session.SetDefaultMark(false);
    //session.SetDefaultTimestampIncrement(480);


}

RemoteInput::~RemoteInput()
{
    stopThread(1000);
}

void RemoteInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    decodedAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    startThread(10);
}
void RemoteInput::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::blue);

}

void RemoteInput::resized()
{

}

void RemoteInput::sendStream(juce::AudioBuffer<float>* soundBuffer)
{
    //UVGRTP
    
    bufferToSend.copyFrom(0, 0, *soundBuffer, 0, 0, soundBuffer->getNumSamples());
    notify();
   /* status = session.SendPacket(cbits, 480);
    DBG("send");
    if (status < 0)
    {
        std::cerr << RTPGetErrorString(status) << std::endl;
        exit(-1);
    }*/


    //frame_size = opus_decode_float(decoder, frame->payload, nbBytes, out, MAX_FRAME_SIZE, 0);



}

void RemoteInput::receiveStream(juce::AudioBuffer<float>* soundBuffer)
{
    if (frame_received)
    {
        soundBuffer->addFrom(0, 0, decodedBuffer, 0, 0, 480);
        soundBuffer->addFrom(1, 0, decodedBuffer, 0, 0, 480);
        frame_received = false;
    }
}


void RemoteInput::run()
{
    while(!threadShouldExit())
    {
        nbBytes = opus_encode_float(encoder, bufferToSend.getReadPointer(0), FRAME_SIZE, cbits, MAX_PACKET_SIZE);
        hevc->push_frame(cbits, nbBytes, clock_rate * timestamp++, RCE_RTCP);
        wait(1000);

        /*uvgrtp::frame::rtp_frame* frame;
        frame = recv->pull_frame();
        frame_size = opus_decode_float(decoder, frame->payload, frame->payload_len, out, 480, 0);
        (void)uvgrtp::frame::dealloc_frame(frame);
        decodedBuffer.copyFrom(0, 0, out, 480);
        if (frame_size > 0)
            frame_received = true;*/

    }
    //while (true)
    //{
    //    /*RTPTime delay(0.010);
    //    RTPTime starttime = RTPTime::CurrentTime();
    //    bool done = false;*/

    //    status = session.SendPacket(cbits, 480);
    //    DBG("send");
    //    if (status < 0)
    //    {
    //        std::cerr << RTPGetErrorString(status) << std::endl;
    //        exit(-1);
    //    }
    //    //
    //    // Inspect incoming data here
    //    //

    //    /*RTPTime::Wait(delay);

    //    RTPTime t = RTPTime::CurrentTime();
    //    t -= starttime;

    //    session.BeginDataAccess();
    //    if (session.GotoFirstSource())
    //    {
    //        do
    //        {
    //            RTPPacket* packet;
    //            while ((packet = session.GetNextPacket()) != 0)
    //            {
    //                DBG("packet received");
    //                session.DeletePacket(packet);
    //            }
    //        } while (session.GotoNextSource());
    //    }
    //    session.EndDataAccess();*/
    //}
}