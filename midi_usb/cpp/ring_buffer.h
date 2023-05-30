#pragma once
#include "common.h"
template<u8 N> class RingBuffer{
private:
    static constexpr u32 MEM_SIZE = 1 << (u32)N;
    static constexpr u32 MASK = (1 << (u32)N) - 1;
    volatile u8 buffer[MEM_SIZE] ALIGNMENT_CACHELINE;
    volatile u32 start;
    volatile u32 size; // CS
    volatile u32 end;
public:
    RingBuffer(){
    	start=0;
        end = 0;
    	size = 0;
    }
    void writeByte(u8 data){
        buffer[end] = data;
        end = (end + 1) & MASK;
        execCriticalSection([&](){
            size++;
        });
    }
    void writeBuffer(u8* buf, u32 len){
        for(unsigned int i = 0; i < len; i++){
            buffer[end] = buf[i];
            end = (end + 1) & MASK;
        }
        execCriticalSection([&](){size += len;});
    }
    unsigned int readBuffer(u8* buf, u32 len){
        execCriticalSection([&](){
            len = len < size ? len : size;
        });
        for(u32 i = 0; i < len; ++i){
            buf[i] = buffer[start];
            start = (start + 1) & MASK;
        }
        execCriticalSection([&](){
            size-=len;
        });
        return len;
    }
    int readByte(){
        if(getLength() == 0) return -1;
        u8 ret;
        ret = buffer[start];
        start = (start + 1) & MASK;
        execCriticalSection([&](){
            size--;
        });
        return ret;
    }
    int operator [](u32 index){
        u8 ret;
        if(index >= getLength()) return -1;
        ret = buffer[(start + index) % MEM_SIZE];
        return ret;
    }
    int findBuffer(u8 target){
        u32 c = start;
        u32 e = getLength();
        for(u32 i = 0; i < e ; i++){
            if(buffer[c] == target){
                return i;
            }
            c = (c + 1) & MASK;
        }
        return -1;
    }

    template<typename F>
    void findBuffer(const F& func){
        u32 c = start;
        u32 e = getLength();
        for(u32 i = 0; i < e ; i++){
            if(!func(buffer[c], i))return;
            c = (c + 1) & MASK;
        }
    }
    
    u32 getLength(){
        u32 ret;
        execCriticalSection([&](){
            ret = size;
        });
        return ret;
    }
};
