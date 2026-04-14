#include <iostream>
#include <cstdlib>

#include "PacketTracer/PacketTracer.h"

int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        perror("not enough arguments");
    }

    PacketTracer tracer;
    tracer.sniffPackets(argv[1]);

    return 0;
}
