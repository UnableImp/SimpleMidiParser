/******************************************************************************/
/*!
\file   Parser.cpp
\author Zack Krolikowksi
\date   12/17/2020
*/
/******************************************************************************/
#include "Parser.h"

#ifdef DEBUG

#include <iostream>
#include <iomanip>

#endif

//----------------------------------------------------------------------
//                        PARSER
//----------------------------------------------------------------------

// all data in midi files is big endian
inline void endian_swap(unsigned short& x)
{
    x = (x>>8) | 
        (x<<8);
}

inline void endian_swap(unsigned int& x)
{
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

FileMidiParser::FileMidiParser(const char* filename) : file(filename, std::ios_base::binary | std::ios_base::in), 
                                                       header(),
                                                       // Default assumed values until set to be something else by meta events
                                                       m_MPQN(500000),     // 120 bpm
                                                       m_bpm(120),         // 120 bpm
                                                       m_TicksPerBeat(0)   // From header
{

}

bool FileMidiParser::ReadHeader()
{
    char read[MidiHeaderSize];
    file.read(read, MidiHeaderSize);

    header.id[0] = read[0];
    header.id[1] = read[1];
    header.id[2] = read[2];
    header.id[3] = read[3];

    header.chunkSize = *reinterpret_cast<unsigned*>(read + 4);
    header.type = *reinterpret_cast<short*>(read + 8);
    header.numTracks = *reinterpret_cast<short*>(read + 10);
    header.timeDivision = *reinterpret_cast<short*>(read + 12);
    
    endian_swap(header.chunkSize);
    endian_swap(header.type);
    endian_swap(header.numTracks);
    endian_swap(header.timeDivision);

    m_TicksPerBeat = header.timeDivision;
    m_bpm = MicroSecondsPerMinute / m_MPQN;


    #ifdef DEBUG
    print();
    #endif

    return true;
}

bool FileMidiParser::ReadTracks()
{
    char track[8];
    file.read(track, 8);
    unsigned size = *reinterpret_cast<unsigned*>(track+4);
    endian_swap(size);

    unsigned char* data = new unsigned char[size];
    file.read(reinterpret_cast<char*>(data), size);

    double totalTime = 0;
    unsigned pos = 0;
    while(pos < size - 1)
    {
        unsigned deltaTime;
        pos += GetVaribaleLengthValue(data + pos, deltaTime);

        double time = m_MPQN * (deltaTime / m_TicksPerBeat) + totalTime;
        totalTime = time;
        
        float beat = (totalTime / MicroSecondsPerMinute) * m_bpm;

        unsigned char event = *(data + pos);
        if(event == 255)
            pos += HandleMetaEvent(data + pos, beat);
        else
            pos += HandleMidiEvent(data + pos, beat);
    }

    delete [] data;

    return true;
}

#ifdef DEBUG
void FileMidiParser::print()
{  
    std::cout << "Header         : " << header.id << std::endl;
    std::cout << "Chunk Size     : " << header.chunkSize << std::endl;
    std::cout << "Type           : " << header.type << std::endl;
    std::cout << "Num Tracks     : " << header.numTracks << std::endl;
    std::cout << "Tim Div        : " << header.timeDivision << std::endl;
    std::cout << "Ticks per beat : " << static_cast<unsigned>(m_TicksPerBeat) << std::endl;
}
#endif

unsigned FileMidiParser::HandleMetaEvent(unsigned char* data, float beat)
{
    #ifdef DEBUG
    std::cout << "---------------NEW META EVENT---------------" << std::endl;
    #endif

    // Offset starts at 1 to accound for Meta Event byte
    int totalOffset = 1;

    unsigned char type = *(data + totalOffset);
    ++totalOffset;

    switch(static_cast<Events::MetaEvents>(type))
    {
        case Events::MetaEvents::TimeSignature:
        {
            // Lenght byte
            ++totalOffset;

            // Numerator byte
            unsigned char numerator = *(data + totalOffset);
            ++totalOffset;

            // Denominator byte, defined as 2^value
            unsigned denominator = 1 << *(data + totalOffset);
            ++totalOffset;

            // Metro byte
            unsigned metro = *(data + totalOffset);
            ++totalOffset;

            // 32nds byte
            unsigned char hits = *(data + totalOffset);
            ++totalOffset;

            SetTimeSignature(numerator, denominator, metro, hits, beat);

            break;
        }

        case Events::MetaEvents::KeySignature:
        {
            // Lenght byte
            ++totalOffset;

            // Key byte
            char key = *(data + totalOffset);
            ++totalOffset;

            // Scale byte
            char scale = *(data + totalOffset);
            ++totalOffset;

            SetKeySignature(key, scale, beat);

            break;
        }

        default:
        {
            #ifdef DEBUG
            std::cout << "Unknown meta event with type: " << static_cast<unsigned>(type) << std::endl;
            #endif
        }

    }

    return totalOffset;
}

unsigned FileMidiParser::HandleMidiEvent(unsigned char* data, float beat)
{
    #ifdef DEBUG
    std::cout << "---------------NEW MIDI EVENT---------------" << std::endl;
    #endif

    // midi event type is first 4 bits of first byte
    unsigned char type = *data >> 4;
    
    // channel is last 4 bits of first byte
    unsigned char channel = *data << 4;
    channel >>= 4;

    switch(static_cast<Events::MidiEvents>(type))
    {

        case Events::MidiEvents::NoteOn:
        {
            unsigned char note = *(data + 1);
            unsigned char vel = *(data + 2);
            
            // vel of 0 is same as note off
            if(vel)
                NoteOn(channel, note, vel, beat);
            else
                NoteOff(channel, note, beat);
            
            break;

        }

        case Events::MidiEvents::NoteOff:
        {
             unsigned char note = *(data + 1);
             NoteOff(channel, note, beat);
             
             break;
        }

        default:
        {
            #ifdef DEBUG
            std::cout << "Unknown midi event with type: " << static_cast<unsigned>(type) << std::endl;
            #endif
            break;
        }
    }
    
    // Size of a midi event is always 3 bytes
    return 3;
}

unsigned FileMidiParser::GetVaribaleLengthValue(unsigned char* data, unsigned& value)
{
    unsigned totalOffset = 1;
    // if MSB is set, more bytes are used to represent data
    while(*(data + totalOffset - 1) & 1 << 7) 
        ++totalOffset; 
    
    value = 0;
    
    // Concate bytes together
    for(int i = totalOffset; i > 0; --i)
    {   
        // Get byte and remove next byte bit
        unsigned num = (*(data + i - 1));
        if(num >> 7)
            num ^= 1 << 7;
      
        // Shift to location in final value and concate together
        value += num << (7 * (totalOffset - i));

    }  

    return totalOffset;
}

#ifdef DEBUG
int main(int argc, char *argv[])
{
    if(argc < 2)
        return 0;

    FileMidiParser parser(argv[1]);
    parser.ReadHeader();
    parser.ReadTracks();
}
#endif
//----------------------------------------------------------------------
//                        META EVENTS
//----------------------------------------------------------------------

void FileMidiParser::SetTimeSignature(unsigned num, unsigned den, unsigned metro, unsigned hits, float beat)
{
    #ifdef DEBUG
    
    std::cout << "Got time Signature" << std::endl;
    std::cout << num << " / " << den << " time" << std::endl;
    std::cout << metro << " metro" << std::endl;
    std::cout << hits << " 32nds" << std::endl;
    std::cout << beat << " beat" << std::endl;

    #endif
}

void FileMidiParser::SetKeySignature(unsigned key, unsigned quality, float beat)
{
    #ifdef DEBUG
 
    std::cout << "Get key signature" << std::endl;
    std::cout << "Key     : " << key << std::endl;
    std::cout << "Quality : " << quality << std::endl;
    std::cout << "beat    : " << beat << std::endl;

    #endif
}

//----------------------------------------------------------------------
//                        MIDI EVENTS
//----------------------------------------------------------------------

void FileMidiParser::NoteOn(unsigned char channel, unsigned char note, unsigned char vel, float beat)
{
    #ifdef DEBUG

    std::cout << "Got note on" << std::endl;
    std::cout << "Channel  : " << static_cast<unsigned>(channel) << std::endl;
    std::cout << "Note     : " << static_cast<unsigned>(note) << std::endl;
    std::cout << "Velocity : " << static_cast<unsigned>(vel) << std::endl;
    std::cout << "beat     : " << static_cast<unsigned>(beat) << std::endl;

    #endif
}

void FileMidiParser::NoteOff(unsigned char channel, unsigned char note, float beat)
{
    #ifdef DEBUG

    std::cout << "Got note off" << std::endl;
    std::cout << "Channel  : " << static_cast<unsigned>(channel) << std::endl;
    std::cout << "Note     : " << static_cast<unsigned>(note) << std::endl;
    std::cout << "beat     : " << beat << std::endl;

    #endif
}