#pragma once

#include <memory>
#include <vector>
#include <cstddef>

namespace prolog::utils {

template<typename T>
class MemoryPool {
private:
    struct Block {
        alignas(T) char data[sizeof(T)];
        bool used = false;
    };
    
    std::vector<std::unique_ptr<Block[]>> chunks_;
    std::vector<size_t> free_indices_;
    size_t chunk_size_;
    size_t current_chunk_;
    size_t current_index_;
    
public:
    explicit MemoryPool(size_t chunk_size = 1024) 
        : chunk_size_(chunk_size), current_chunk_(0), current_index_(0) {
        addChunk();
    }
    
    ~MemoryPool() {
        clear();
    }
    
    template<typename... Args>
    T* allocate(Args&&... args) {
        Block* block = findFreeBlock();
        if (!block) {
            addChunk();
            block = findFreeBlock();
        }
        
        block->used = true;
        return new(block->data) T(std::forward<Args>(args)...);
    }
    
    void deallocate(T* ptr) {
        if (!ptr) return;
        
        ptr->~T();
        
        for (size_t chunk_idx = 0; chunk_idx < chunks_.size(); ++chunk_idx) {
            auto& chunk = chunks_[chunk_idx];
            ptrdiff_t offset = reinterpret_cast<char*>(ptr) - reinterpret_cast<char*>(chunk.get());
            
            if (offset >= 0 && offset < static_cast<ptrdiff_t>(chunk_size_ * sizeof(Block))) {
                size_t block_idx = offset / sizeof(Block);
                chunk[block_idx].used = false;
                free_indices_.push_back(chunk_idx * chunk_size_ + block_idx);
                return;
            }
        }
    }
    
    void clear() {
        for (auto& chunk : chunks_) {
            for (size_t i = 0; i < chunk_size_; ++i) {
                if (chunk[i].used) {
                    reinterpret_cast<T*>(chunk[i].data)->~T();
                }
            }
        }
        chunks_.clear();
        free_indices_.clear();
        current_chunk_ = 0;
        current_index_ = 0;
    }
    
    size_t totalCapacity() const {
        return chunks_.size() * chunk_size_;
    }
    
    size_t usedCount() const {
        size_t used = 0;
        for (const auto& chunk : chunks_) {
            for (size_t i = 0; i < chunk_size_; ++i) {
                if (chunk[i].used) {
                    ++used;
                }
            }
        }
        return used;
    }
    
private:
    void addChunk() {
        chunks_.push_back(std::make_unique<Block[]>(chunk_size_));
        current_chunk_ = chunks_.size() - 1;
        current_index_ = 0;
    }
    
    Block* findFreeBlock() {
        // First try free indices
        if (!free_indices_.empty()) {
            size_t global_idx = free_indices_.back();
            free_indices_.pop_back();
            
            size_t chunk_idx = global_idx / chunk_size_;
            size_t block_idx = global_idx % chunk_size_;
            
            return &chunks_[chunk_idx][block_idx];
        }
        
        // Then try current chunk
        while (current_chunk_ < chunks_.size()) {
            auto& chunk = chunks_[current_chunk_];
            
            while (current_index_ < chunk_size_) {
                if (!chunk[current_index_].used) {
                    return &chunk[current_index_++];
                }
                current_index_++;
            }
            
            current_chunk_++;
            current_index_ = 0;
        }
        
        return nullptr;
    }
};

}