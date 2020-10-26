#include <fstream>
#include <iostream>

class customPrint
{
    std::ofstream* output_file_;
public:

    customPrint(std::ofstream* output_file)
    {
        output_file_ = output_file;
    }

    void print(std::string msg)
    {
        std::cout << msg << std::endl;
        *output_file_ << msg << std::endl;
    }
};