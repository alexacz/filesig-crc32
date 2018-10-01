//
//  main.cpp
//  veeam_sig
//
//  Created by Alex Nik on 30/09/2018.
//

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <limits>
#include <string>
#include <stdlib.h>
#include "Buffer.h"
#include "Reader.h"
#include "Writer.h"
#include "Processor.h"

/**
 *  Display usage
 */
void display_usage()
{
    std::cout << "\n\r";
    std::cout << "veeam_sig [input_file] [output_file] [block_size * in bytes *]\n\r";
}

/**
 *  Parse arguments
 */
bool parse_args(  int argc
                , const char * argv[]
                , std::string& sourceFile
                , std::string& outputFile
                , size_t& blockSize )
{
    if ( argc >= 3 )
    {
        sourceFile = argv[1];
        outputFile = argv[2];
        blockSize = (size_t)( argc == 4 ? atoi(argv[3]) : blockSize );
        return true;
    }
    return false;
}

/**
 *  Main
 */
int main(int argc, const char * argv[]) {
    
    std::string sourceFileName;
    std::string outputFileName;
    size_t      blockSize = BLOCK_SIZE;    //!< default block size in bytes
    
    // Input arguments are reguired.
    // Gets source file name, output file name and block size (optional).
    if ( parse_args(argc, argv, sourceFileName, outputFileName, blockSize) == false )
    {
        display_usage();
        exit(EXIT_SUCCESS);
    }
    
    auto start_time = std::chrono::system_clock::now();
    size_t blocks   = 0;

    {
        std::ifstream   source(sourceFileName, std::ios_base::binary | std::ios_base::in );
        std::ofstream   output(outputFileName, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc );
    
        if ( source.is_open() && output.is_open() )
        {
            Buffer          buffer(blockSize);
            Reader          reader(buffer,source);
            Writer          writer(buffer,output);
            Processor       processor1(buffer,writer), processor2(buffer,writer);
            
            std::cout << "Calculates CRC32..." << "\n";

            std::thread     producer_thread(&Reader::run,&reader);
            std::thread     consumer_thread1(&Processor::run,&processor1);
            std::thread     consumer_thread2(&Processor::run,&processor2);

            producer_thread.join();
            consumer_thread1.join();
            consumer_thread2.join();

            blocks = writer.flush();
        }
        else
        {
            if ( source.is_open() ) std::cout << "Cannot open file : " << sourceFileName << std::endl;
            if ( output.is_open() ) std::cout << "Cannot open file : " << outputFileName << std::endl;
        }
    }

    auto diff = std::chrono::system_clock::now() - start_time;
    auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    
    std::cout << "Program run   : " << seconds.count() << " [ms]" << std::endl;
    std::cout << "Blocks saved  : " << blocks << std::endl;
    std::cout << "Block size    : " << blockSize << " [bytes]" << std::endl;
    std::cout << "Sources file  : " << sourceFileName << std::endl;
    std::cout << "Signature file: " << outputFileName << std::endl;

    return 0;
}
