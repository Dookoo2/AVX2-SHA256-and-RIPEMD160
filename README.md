# 🚀 Hash Libraries: SHA-256 and RIPEMD-160

This repository contains highly optimized implementations of the **SHA-256 (AVX2 and SHA extensions)** and **RIPEMD-160 (AVX2)** hash functions, designed for maximum performance. These libraries achieve exceptional speed and are suitable for high-throughput cryptographic applications such as blockchain, data integrity verification, and research.  
The folders contain example programs that compute:  
**SHA-256:** The SHA-256 hash of a Bitcoin compressed public key.  
**RIPEMD-160:** The RIPEMD-160 hash derived from the SHA-256 hash.  

---

## 🌟 **Performance Highlights**

The implementations were tested on a **virtual machine** running **Ubuntu 24** with the following specifications:
1. **Processor**: AMD Ryzen 7 5800H  (8 cores, 16 threads)
2. **RAM**: 16 GB  (2 channels)

---

## ⚡ **Features**

- **High Performance**: Optimized using **AVX2** for SIMD acceleration.
- **Cross-Platform**: Tested on Linux (Ubuntu 24) and can be compiled and run on other platforms with minimal modifications.
- **Customizability**: The codebase is modular and can be extended or integrated into other projects.
- **Ease of Integration**: Simple to integrate into other projects.

---

## 📊 **Results Comparison**

| Algorithm               | Total Hashes    | Total Time (s) | Avg Time per Hash (ns) | Throughput (Mhash/s) |
|-------------------------|-----------------|----------------|------------------------|----------------------|
| **SHA-256 (AVX2)**      | 1,000,000,000   | 6.75           | 6.75                   | ~147                 |
| **RIPEMD-160 (AVX2)**   | 1,000,000,000   | 3.55           | 3.55                   | ~281                 |
| **SHA-256 (Extensions)**| 1,000,000,000   | **3.55**       | **2.24**               | **~446**             |

---

## 🛠️ **Compile the Code**

To compile, use the following commands:

```bash
# For SHA-256 (AVX2)
g++ -O3 -mavx2 -fopenmp -std=c++17 sha256_avx2_gen.cpp sha256_avx2.cpp -o sha256

# For RIPEMD-160 (AVX2)
g++ -O3 -mavx2 -fopenmp -std=c++17 ripemd160_avx2_gen.cpp ripemd160_avx2.cpp -o ripemd160

# For SHA-256 (Extensions)
g++ -O3 -msha -msse4.1 -fopenmp -std=c++17 -flto sha256_ext_gen.cpp sha256_ext.cpp -o sha256_ext
```

---

## ✌️**TIPS**
BTC: bc1qtq4y9l9ajeyxq05ynq09z8p52xdmk4hqky9c8n

