//
//  Buffer.h
//  veeam_sig
//
//  Created by Alex Nik on 30/09/2018.
//

#ifndef buffer_h
#define buffer_h

#include <memory>
#include <mutex>
#include <deque>
#include <condition_variable>

// Default block size 1MB
#define BLOCK_SIZE  (1024*1024)

/**
 *  \class Buffer
 *  Collects blocks to buffer from producer for further processing of consumers.
 */
class Buffer {
    
public:

    struct Block {
        enum { Last = std::numeric_limits<size_t>::max() };
        size_t                      _num;
        size_t                      _len;
        std::unique_ptr<uint8_t[]>  _data;
        explicit Block( size_t num, size_t len):_num(num),_len(len),_data(new uint8_t[len]) {;}
    };
    
    /**
     *  Constructor.
     *  \param size The size of block.
     */
    Buffer(size_t size = BLOCK_SIZE):_block_size(size),_finished(false) {;}
    
    /**
     *  Allocates a block.
     *  \return Ptr to block, otherwise exception is occurred.
     */
    std::shared_ptr<Block> allocate()
    {
        static size_t block_id = 0;
        std::shared_ptr<Buffer::Block> block;
        try {
            block =  std::make_shared<Buffer::Block>(block_id++,_block_size);
        } catch (const std::bad_alloc& e) {
            bad_alloc_process();
        }
        return block;
    }
    
    /**
     *  Adds block to internal buffer from producer and notify the consumers.
     *  \param block Ptr to block.
     *  \param last The last block marker.
     */
    void add(const std::shared_ptr<Block>& block, bool last = false )
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if ( last ) block->_num = Block::Last;
        try {
            _buffer.push_back(block);
            _cond.notify_all();
        } catch (const std::bad_alloc& e) {
            bad_alloc_process();
        }
    }
    
    /**
     *  Gets block from internal buffer.
     *  \param block Ptr to block.
     *  \return True if the block is exctacted successfully, otherwise the last block was already processed.
     */
    bool get(std::shared_ptr<Block>& block)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        
        while ( _buffer.empty() && _finished == false )
            _cond.wait(lock);
        
        bool ret = !_finished;
        
        if ( _finished == false )
        {
            block = _buffer.front();
            _buffer.pop_front();
            if ( block->_num == Block::Last ) _finished = true;
        }
        
        return ret;
    }
    
private:
    
    void bad_alloc_process()
    {
        _finished = true;       // mark the flag to finish
        _cond.notify_all();     // notify threads
        throw std::bad_alloc(); // send exception to owner
    }
    
private:
    std::deque<std::shared_ptr<Block>>  _buffer;
    size_t                              _block_size;
    std::mutex                          _mutex;
    std::condition_variable             _cond;
    bool                                _finished;  // Indicates that all data in buffer was processed.
};

#endif /* buffer_h */
