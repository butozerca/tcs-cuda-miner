#pragma once

#include<vector>
#include<string>

class Miner {
    public:
        virtual int mine(const char *input, int nonce_begin, int nonce_end, int difficulty) = 0;
};
