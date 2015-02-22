#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#define BASE_OFFSET 256
#define THREAD_SIZE 564

#define M_BLOCK_OFFSET 0
#define M_H_OFFSET 128
#define W_OFFSET 160
#define WV_OFFSET 416
#define NONCE_INPUT_OFFSET 448
#define DIGEST_OFFSET 528

#define THREAD_VAR(offset) (shared_mem + BASE_OFFSET + THREAD_SIZE * threadIdx.x + (offset))

#define M_BLOCK THREAD_VAR(M_BLOCK_OFFSET)
#define M_H ((unsigned int *)(THREAD_VAR(M_H_OFFSET)))
#define W ((unsigned int *)(THREAD_VAR(W_OFFSET)))
#define WV ((unsigned int *)(THREAD_VAR(WV_OFFSET)))
#define NONCE_INPUT ((unsigned char*)(THREAD_VAR(NONCE_INPUT_OFFSET)))
#define DIGEST THREAD_VAR(DIGEST_OFFSET)

 
class SHA256
{
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;
 
public:
    __device__
    void init(char* shared_mem);
    __device__
    void update(const unsigned char *message, unsigned int len, char* shared_mem);
    __device__
    void final(unsigned char *digest, char* shared_mem);
 
protected:
    __device__
    void transform(const unsigned char *message, unsigned int block_nb, char* shared_mem);
    unsigned int m_tot_len;
    unsigned int m_len;
};

__device__
void sha256(const char* s, int length, char* out);
 
#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}

__device__
void SHA256::transform(const unsigned char *message, unsigned int block_nb, char* shared_mem)
{
    uint32* w = W;
    uint32* wv = WV;
    unsigned int* m_h = M_H;
    unsigned int* sha256_k = (unsigned int*)shared_mem;
    uint32 t1, t2;
    const unsigned char *sub_block;
    int i;
    int j;
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
        for (j = 0; j < 16; j++) {
            SHA2_PACK32(&sub_block[j << 2], &w[j]);
        }
        for (j = 16; j < 64; j++) {
            w[j] =  SHA256_F4(w[j -  2]) + w[j -  7] + SHA256_F3(w[j - 15]) + w[j - 16];
        }
        for (j = 0; j < 8; j++) {
            wv[j] = m_h[j];
        }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) {
            m_h[j] += wv[j];
        }
    }
}

__device__
void SHA256::init(char* shared_mem)
{
    unsigned int *m_h = M_H;
    m_h[0] = 0x6a09e667;
    m_h[1] = 0xbb67ae85;
    m_h[2] = 0x3c6ef372;
    m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f;
    m_h[5] = 0x9b05688c;
    m_h[6] = 0x1f83d9ab;
    m_h[7] = 0x5be0cd19;
    m_len = 0;
    m_tot_len = 0;
}

__device__
void SHA256::update(const unsigned char *message, unsigned int len, char* shared_mem)
{
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    char* m_block = M_BLOCK;
    tmp_len = 64 - m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&m_block[m_len], message, rem_len);
    if (m_len + len < 64) {
        m_len += len;
        return;
    }
    new_len = len - rem_len;
    block_nb = new_len / 64;
    shifted_message = message + rem_len;
    transform((unsigned char*)m_block, 1, shared_mem);
    transform(shifted_message, block_nb, shared_mem);
    rem_len = new_len % 64;
    memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
    m_len = rem_len;
    m_tot_len += (block_nb + 1) << 6;
}

__device__
void SHA256::final(unsigned char *digest, char* shared_mem) {
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    char* m_block = M_BLOCK;
    int i;
    block_nb = (1 + ((64 - 9)
                     < (m_len % 64)));
    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;
    memset(m_block + m_len, 0, pm_len - m_len);
    m_block[m_len] = 0x80;
    SHA2_UNPACK32(len_b, m_block + pm_len - 4);
    transform((unsigned char*)m_block, block_nb, shared_mem);
    for (i = 0 ; i < 8; i++) {
        SHA2_UNPACK32(M_H[i], &digest[i << 2]);
    }
}

__device__
void sha256(const char* input, int length, char* output, char* shared_mem)
{
    SHA256 ctx;
    ctx.init(shared_mem);
    ctx.update( (unsigned char*)input, length, shared_mem);
    ctx.final((unsigned char*)output, shared_mem);
}

extern "C" {
__global__ 
void Gpu_hash(const char* input, const unsigned int* sha_const, int length, int nonce_offset, int difficulty, int* result)
{
    __shared__ char shared_mem[49152];

    if (threadIdx.x == 0)
        for (int i = 0; i < 64; ++i)
            ((unsigned int*)shared_mem)[i] = sha_const[i];
    
    __syncthreads();

    const int nonce = nonce_offset + blockIdx.x*blockDim.x+threadIdx.x;
    
    unsigned char* nonce_input = NONCE_INPUT;
    for (int i = 0; i < length; ++i)
        nonce_input[i] = input[i];
    memcpy(nonce_input + length, (void*)&nonce, 4);

    unsigned char* digest = (unsigned char*)DIGEST;
 
    SHA256 ctx = SHA256();
    ctx.init(shared_mem);
    ctx.update(nonce_input, length + 4, shared_mem);
    ctx.final(digest, shared_mem);
    
    /*for (int i = 0; i < blockDim.x; ++i) {
        if(i != threadIdx.x) continue;
        printf("gpu hashed once:\n");
        for(int i = 0; i < 8; ++i) {
            printf("%08x", ((int*)digest)[i]);
        }printf("\n");
    }*/

    ctx.init(shared_mem);
    ctx.update(digest, 32, shared_mem);
    ctx.final(nonce_input, shared_mem);
    
   /* for (int i = 0; i < blockDim.x; ++i) {
        if(i != threadIdx.x) continue;
        printf("gpu hashed twice:\n");
        for(int i = 0; i < 8; ++i) {
            printf("%08x", ((int*)nonce_input)[i]);
        }printf("\n");
    }*/

    for (int i = 0; i < (difficulty >> 3); ++i)
        if (nonce_input[i] != 0) return;
    if (nonce_input[difficulty >> 3] <= 255 >> (difficulty & 7)) {
        *result = nonce;
    }
}
}




/*
 * Updated to C++, zedwood.com 2012
 * Based on Olivier Gay's version
 * See Modified BSD License below: 
 *
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Issue date:  04/30/2005
 * http://www.ouah.org/ogay/sha2/
 *
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


