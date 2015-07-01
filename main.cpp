#include<chrono>
#include<climits>
#include<cstdio>
#include<cstring>
#include<fstream>
#include<iostream>
#include<string>
#include<vector>
#include<cstdlib>

#include "Cpu_miner.hpp"
#include "Gpu_miner.hpp"
#include "sha256.hpp"

const std::string file_in = "input.txt";


void confirm_nonce(const char *data, int nonce, int difficulty)
{
	unsigned char data2[80], hash[32], hash2[32];

	memcpy(data2, data, 76);
	*reinterpret_cast<int *>(data2 + 76) = nonce;

	SHA256 sha;
	sha.init();
	sha.update(data2, 80);
	sha.final(hash);

	sha.init();
	sha.update(hash, 32);
	sha.final(hash2);

	char hash_str[65];
	for(int i = 0; i < 32; ++i)
		sprintf(hash_str + 2 * i, "%02x", hash2[31 - i]);
	hash_str[64] = 0;

	std::cout << "Policzony hash " << hash_str << std::endl;

	--difficulty;
	char cmp_str[65];

	for(int i = 0; i < difficulty / 4; ++i)
		cmp_str[i] = '0';

	cmp_str[difficulty / 4] = '0' + (8 >> (difficulty % 4));
	cmp_str[difficulty / 4 + 1] = 0;

	if(strcmp(hash_str, cmp_str) < 0)
		std::cout << "Dobry hash" << std::endl;
	else
		std::cout << "Zły hash" << std::endl;
}

int main(int argc, char** argv)
{

    std::ifstream input(file_in);
    
    std::vector<std::string> in;

    while(input.good())
    {
        std::string s;
        input >> s;
        if (s.size() != 76) break;
        in.push_back(s);
    }

    if(in.empty())
    {
        std::cout << "nothing to hash\n";
        return 1;
    }

    Cpu_miner cpu_miner;
    Gpu_miner gpu_miner;
    
    int difficulty = 24;
    int min_nonce = 0, max_nonce = 40000000;
    
    if (argc > 1) {
        difficulty = atoi(argv[1]);
        printf("difficulty set to %d\n", difficulty);
    }

    if (argc > 3) {
        min_nonce = atoi(argv[2]);
        max_nonce = atoi(argv[3]);
        printf("nonce interval set to %d %d\n", min_nonce, max_nonce);
    }


    for (auto &s: in) {
        std::cout << "hash: " << s << std::endl;
        std::cout << "CPU PART ----------" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        int res = cpu_miner.mine(s.c_str(), min_nonce, max_nonce, difficulty);
        auto finish = std::chrono::high_resolution_clock::now();
        printf("nonce=%d\n", res);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        std::cout << "mining took " << milliseconds.count() << "ms\n";
        printf("speed: %ld KHash/s\n", ((((res == -1)?max_nonce:res) - min_nonce)/(milliseconds.count()+1)));
        confirm_nonce(s.c_str(), res, difficulty);
        

        std::cout << "GPU PART ----------" << std::endl;
        start = std::chrono::high_resolution_clock::now();
        res = gpu_miner.mine(s.c_str(), min_nonce, max_nonce, difficulty);
        finish = std::chrono::high_resolution_clock::now();
        printf("nonce=%d\n", res);
        milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish-start);
        std::cout << "mining took " << milliseconds.count() << "ms\n";
        printf("speed: %ld KHash/s\n", ((((res == -1)?max_nonce:res) - min_nonce)/(milliseconds.count()+1)));
        confirm_nonce(s.c_str(), res, difficulty);

        printf("\n");
    }

    return 0;
}
