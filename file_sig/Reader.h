//
//  Reader.h
//  veeam_sig
//
//  Created by Alex Nik on 30/09/2018.
//

#ifndef reader_h
#define reader_h

#include <algorithm>
#include "Buffer.h"

/**
 *  Reads data from input stream by blocks (fixed length)
 *  and collects the blocks to buffer.
 */
class Reader {
    
    Buffer&         _buffer;
	std::ifstream&  _stream;

public:
    
    /**
     *  \param buffer Reference to Buffer.
     *  \param stream Reference to input file stream.
     *  \note The input file stream must be opened before.
     */
    Reader( Buffer& buffer, std::ifstream& stream ):
    _buffer(buffer)
    ,_stream(stream)
    {;}
    
    
    /**
     *  Main loop
     */
    void run()
    {
        auto last_block = false;
        
        if ( _stream.is_open() )
        {
            
            while ( !last_block )
            {
                try {
                    
                    auto block = _buffer.allocate();
                    
                    // read data to block
                    _stream.read( reinterpret_cast<char*>( block->_data.get() ), block->_len );
                    
                    // get a number of extracted characters
#ifdef _WIN32
					auto read = (size_t)_stream.gcount();
#else
                    auto read = (size_t)_stream.gcount();
#endif                    
                    // if number of extracted characters is less then block size, it is the last block
                    if ( read < block->_len )
                    {
                        auto ptr = &block->_data[read];
                        
                        auto len = block->_len - read;
                        
                        std::fill_n(ptr,len,0);
                        
                        last_block = true;
                    }
                    
                    _buffer.add(block, last_block);

                }
                catch (const std::exception& e) {
                    std::cout << "Reader is occurred exception: " << e.what() << std::endl;
                    break;
                }
            }
            
        }
        else
        {
            std::cout << "Cannot read file" << std::endl;
        }
        
    }
    
private:
};

#endif /* reader_h */
