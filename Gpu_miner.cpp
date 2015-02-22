#include <cuda.h>

#include "Gpu_miner.hpp"

//TODO handle difficulty and input not being a string
int Gpu_miner::mine(const char *input, int nonce_begin, int nonce_end, int difficulty)
{
    cuInit(0);
    
    CUdevice cuDevice;
    CUresult res = cuDeviceGet(&cuDevice, 0);
    if (res != CUDA_SUCCESS){
        printf("cannot acquire device 0\n"); 
        exit(1);
    }

    CUcontext cuContext;
    res = cuCtxCreate(&cuContext, 0, cuDevice);
    if (res != CUDA_SUCCESS){
        printf("cannot create context\n");
        exit(1);
    }

    CUmodule cuModule = (CUmodule)0;
    res = cuModuleLoad(&cuModule, "Gpu_miner.ptx");
    if (res != CUDA_SUCCESS) {
        printf("cannot load module: %d\n", res);  
        exit(1); 
    }

    CUfunction Gpu_hash;
    res = cuModuleGetFunction(&Gpu_hash, cuModule, "Gpu_hash");
    if (res != CUDA_SUCCESS){
        printf("cannot acquire kernel handle\n");
        exit(1);
    }
	
    int gridX=1024;
	int gridY=1;
    int gridZ=1;
	int blockX=80;
	int blockY=1;
    int blockZ=1;

    int batch_size = gridX * blockX;
    
    unsigned int sha256_k[64] = //UL = uint32
            {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
             0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
             0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
             0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
             0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
             0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
             0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
             0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
             0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
             0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
             0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
             0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
             0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
             0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
             0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    
    CUdeviceptr gpu_data, sha_const;
    int data_size=76;
    char* data = const_cast<char*>(input);

    cuMemHostRegister(sha256_k, 256, 0);
    
    cuMemAlloc(&sha_const, 256);
    cuMemcpyHtoD(sha_const, sha256_k, 256);
    
    cuMemHostRegister(data, data_size, 0);
    
    cuMemAlloc(&gpu_data, data_size);
    cuMemcpyHtoD(gpu_data, data, data_size);

    CUdeviceptr gpu_result;
    cuMemAlloc(&gpu_result, 4);
    int host_result[1]={-1};
    cuMemHostRegister(host_result, 4, 0);
    cuMemcpyHtoD(gpu_result, host_result, 4);
    
    for (int batch_offset = nonce_begin; batch_offset < nonce_end; batch_offset += batch_size) {
        
        void* args[] = {&gpu_data, &sha_const, &data_size, &batch_offset, &difficulty, &gpu_result};
        res = cuLaunchKernel(Gpu_hash, gridX, gridY, gridZ, blockX, blockY, blockZ, 0, 0, args, 0);
        
        if (res != CUDA_SUCCESS){
            printf("cannot run kernel\n");
            exit(1);
        }
	    res = cuCtxSynchronize();
        if (res != 0) printf("kuda sie wypierdolila 1 %d\n", res);
	    cuMemcpyDtoH(host_result, gpu_result, 4);
        if (host_result[0] != -1)
            break;
    }

    
    res = cuCtxDestroy(cuContext);
    return host_result[0];
}
