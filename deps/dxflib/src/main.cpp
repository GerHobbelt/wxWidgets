#include <iostream>

#include "DxfConverter.h"

void usage();
void testReading(const char* input, const char* output);

int main(int argc, char** argv) {
    if (argc < 3) {
        usage();
        return 0;
    }

    testReading(argv[1], argv[2]);

    return 0;
}

void usage() {
    std::cout << "\nUsage: test <DXF file> <output file>\n\n";
}

void testReading(const char* input, const char* output) {
    // Load DXF file into memory:
    std::cout << "Reading file " << input << "...\n";
    DxfConverter reader(input, output);
    std::cout << "done" << std::endl;
}

