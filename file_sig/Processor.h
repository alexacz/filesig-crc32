//
//  Processor.h
//  veeam_sig
//
//  Created by Alex Nik on 30/09/2018.
//

#ifndef Processor_h
#define Processor_h

#include "Buffer.h"

extern "C" { uint32_t crc32(uint32_t, const void*, size_t); }

/**
 *  Calculates CRC32 of block and puts the CRC32 to ouput stream.
 */
class Processor {
    
    Buffer&         _buffer;
    Writer&         _writer;
    
public:
    
    /**
     *  Constructor.
     *  \param buffer Reference to Buffer.
     *  \param writer Reference to Writer.
     */
    Processor(Buffer& buffer, Writer& writer):_buffer(buffer),_writer(writer){;}
    
    /**
     *  Main loop
     */
    void run()
    {
        while (true)
        {
            std::shared_ptr<Buffer::Block> block;
            
            if ( _buffer.get(block) )
            {
                try {
                    
                    uint32_t crc = crc32(0, reinterpret_cast<const void*>( block->_data.get() ), block->_len);
                    _writer.add(block->_num,crc);
                    
                } catch (const std::exception& e) {
                    std::cout << "Processor is occurred exception: " << e.what() << std::endl;
                    break;
                }
            }
            else // Last block was processed, exit from while
            {
                break;
            }
            
        } // while (true)
        
    }
};
#endif /* Processor_h */
