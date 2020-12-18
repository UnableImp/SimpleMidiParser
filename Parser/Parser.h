/******************************************************************************/
/*!
\file   Parser.h
\author Zack Krolikowksi
\date   12/17/2020
*/
/******************************************************************************/
#ifndef PARSER_H
#define PARSER_H
#include <fstream>


#define DEBUG

constexpr unsigned MidiHeaderSize = 14;
constexpr unsigned MicroSecondsPerMinute = 60000000;

// TODO SAFTY CHECKS

class FileMidiParser
{
public:
    FileMidiParser(char const* filename);

    bool ReadHeader();

    bool ReadTracks();

    #ifdef DEBUG
    void print();
    #endif

    // Callbacks, simple implenations exists so user only has to take the ones they care about

    //----------------------------------------------------------------------
    //                        META EVENTS
    //----------------------------------------------------------------------
    virtual void SetTimeSignature(unsigned num, unsigned den, unsigned metro, unsigned hits, float beat);
    // TODO perhaps make this an enum? 
    virtual void SetKeySignature(unsigned key, unsigned quality, float beat);

    
    //----------------------------------------------------------------------
    //                        MIDI EVENTS
    //----------------------------------------------------------------------
    virtual void NoteOn(unsigned char channel, unsigned char note, unsigned char vel, float beat);
    virtual void NoteOff(unsigned char channel, unsigned char note, float beat);

private:

    // meta and midi event values - https://web.archive.org/web/20141227205754/http://www.sonicspot.com:80/guide/midifiles.html
    struct Events
    {
        enum MetaEvents
        {
            TimeSignature = 88,
            KeySignature = 89
        };
        enum MidiEvents
        {
            NoteOff = 8,
            NoteOn = 9
        };
    };

    unsigned GetVaribaleLengthValue(unsigned char* data, unsigned& value);
    unsigned HandleMetaEvent(unsigned char* data, float beat);
    unsigned HandleMidiEvent(unsigned char* data, float beat);

    std::fstream file;

    struct MidiHeader
    {
        char id[4];
        unsigned chunkSize;
        unsigned short type;
        unsigned short numTracks;
        unsigned short timeDivision;
    } header;


    unsigned      m_MPQN;           //!< Microseconds per quater note
    unsigned char m_bpm;            //!< beats per minute
    unsigned char m_TicksPerBeat;   //!< Midi ticks per beat

};



#endif