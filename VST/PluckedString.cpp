#include "../Filters/PluckedString.hpp"
#include "../Utils/SaveWave.hpp"

#include <iostream>
#include <ctime>
#include <vector>

int main(int argc, char* argv[])
{
    if(argc < 5)
        return 0;

    // Read args
    float f, r, l; 
    sscanf(argv[1], "%f", &f);
    sscanf(argv[2], "%f", &r);
    sscanf(argv[3], "%f", &l);

    PluckedString<float> pluck(r, f, 0.99985f);

    srand(time(NULL));

    std::vector<float> values;
    values.reserve(r*l);
    
    // Generate data
    for(int i = 0; i < l * r; ++i)
    {
        if(i < 100)
            values.push_back(pluck((rand() % 30001) - 15000));
        else
            values.push_back(pluck(0));
    }

    SaveFile<float>(values, argv[4], r, 1);

    return 0;
}