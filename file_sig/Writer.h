//
//  Writer.h
//  veeam_sig
//
//  Created by Alex Nik on 30/09/2018.
//

#ifndef writer_h
#define writer_h

#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include "Buffer.h"

/**
 *  \class Writer
 *  Collects CRC32 of blocks and writes block to output stream.
 */
class Writer {
    
    Buffer&                                 _buffer;
    std::ofstream&                          _stream;
    std::list<std::pair<size_t, uint32_t>>  _crc32;
    std::mutex                              _mutex;
    
public:
    
    /**
     *  Constuctor.
     *  \param buffer Reference to Buffer.
     *  \param stream Reference to output file stream.
     *  \note The output file stream must be opened before.
     */
    Writer(Buffer& buffer, std::ofstream& stream):
    _buffer(buffer)
    ,_stream(stream) {;}
    
    /**
     *  Adds CRC32 of block to output stream.
     *  \param num  Block number.
     *  \param crc32 CRC32 of block.
     */
    void add(size_t num, uint32_t crc32)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _crc32.push_back(std::pair<size_t, uint32_t>(num,crc32));
    }
    
    
    /**
     *  Flush data to output stream.
     *  \return Number of written blocks.
     */
    size_t flush()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        
        // sort blocks by number
        _crc32.sort([](const std::pair<size_t, uint32_t>& a, const std::pair<size_t, uint32_t>& b) { return a.first < b.first; } );
        
        // transform list to vector
        std::vector<uint32_t> vec;
        vec.reserve(_crc32.size());
        std::transform(_crc32.begin(),
                       _crc32.end(),
                       std::back_inserter(vec),
                       [](const std::pair<size_t, uint32_t>& p) { return p.second; });
        
        if ( _stream.is_open() )
            _stream.write( reinterpret_cast<char*>( vec.data() ), vec.size() * sizeof(std::vector<uint32_t>::value_type));
        
        return vec.size();
    }
    
};

#endif /* writer_h */
