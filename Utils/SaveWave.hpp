#ifndef SAVEWAVE_HPP
#define SAVEWAVE_HPP

#include <fstream>

struct fileheader
{
	char riff_label[4]; // (00) = {'R','I','F','F'}
	unsigned riff_size; // (04) = 36 + data_size
	char file_tag[4]; // (08) = {'W','A','V','E'}
	char fmt_label[4]; // (12) = {'f','m','t',' '}
	unsigned fmt_size; // (16) = 16
	unsigned short audio_format; // (20) = 1
	unsigned short channel_count; // (22) = 1 or 2
	unsigned sampling_rate; // (24) = (anything)
	unsigned bytes_per_second; // (28) = (see above)
	unsigned short bytes_per_sample; // (32) = (see above)
	unsigned short bits_per_sample; // (34) = 8 or 16
	char data_label[4]; // (36) = {'d','a','t','a'}
	unsigned data_size; // (40) = # bytes of data
};

template<typename T>
void SaveFile(std::vector<T> song, char* filename, unsigned samplingRate, unsigned short channelCount)
{
    fileheader header = { {'R','I','F','F'},             
                            static_cast<unsigned>(36 + (static_cast<int>(song.size()) * 2)),
                            {'W','A','V','E'},
                            {'f','m','t',' '},
                            16,
                            1,             
                            channelCount,
                            samplingRate,
                            samplingRate*2*channelCount,
                            static_cast<unsigned short>(2 * channelCount),
                            static_cast<unsigned short>(16 * channelCount),
                            {'d','a','t','a'},
                            static_cast<unsigned>(static_cast<int>(song.size()) * 2)
                        };

    short *data = new short[song.size()];
    for(unsigned i = 0; i < song.size(); ++i) 
        data[i] = static_cast<short>(song[i]);

    std::fstream out(filename, std::ios_base::binary|std::ios_base::out);
    out.write(reinterpret_cast<char*>(&header), 44);
    out.write(reinterpret_cast<char*>(data),song.size() * 2);
    out.close();

    delete [] data;
}

#endif