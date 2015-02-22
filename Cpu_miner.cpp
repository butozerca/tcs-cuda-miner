#include <cstring>
#include "Cpu_miner.hpp"
#include "sha256.hpp"

namespace cpu_miner {

inline bool is_hash_valid(unsigned char* hash, int difficulty) {
    for(int i = 0; i < (difficulty >> 3); ++i)
        if (hash[i] != 0) return false;
    return hash[difficulty>>3] <= (255 >> (difficulty & 7) );
}

}


int Cpu_miner::mine(const char *input, int nonce_begin, int nonce_end, int difficulty)
{   
    char in[80];
    memcpy(in, input, 76);

    for (int j = nonce_begin; j < nonce_end; j++) {
        memcpy(in + 76, (const void*)&j, 4);
    
        char buf[32];
        sha256(in, 80, buf);
        sha256(buf, 32, buf);
        if (cpu_miner::is_hash_valid((unsigned char*)buf, difficulty))
            return j;
    }
    return -1;
}


