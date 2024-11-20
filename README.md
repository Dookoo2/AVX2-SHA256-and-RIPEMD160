# 🚀 Fast Hash Libraries: SHA-256 and RIPEMD-160

This repository contains highly optimized implementations of the **SHA-256** and **RIPEMD-160** hash functions, designed for maximum performance using **AVX2**. These libraries achieve exceptional speed and are suitable for high-throughput cryptographic applications such as blockchain, data integrity verification, and research.

---

## 🌟 **Performance Highlights**

The implementations were tested on a **virtual machine** running **Ubuntu 24** with the following specifications:
1. **Processor**: AMD Ryzen 7 5800H  
2. **RAM**: 16 GB  

---

## ⚡ **Features**

- **High Performance**: Optimized using **AVX2** for SIMD acceleration.
- **Cross-Platform**: Tested on Linux (Ubuntu 24) and can be compiled and run on other platforms with minimal modifications.
- **Customizability**: The codebase is modular and can be extended or integrated into other projects.
- **Ease of Integration**: Simple to integrate into other projects.

---

## 📊 **Results Comparison**

| Algorithm     | Total Hashes    | Total Time (s) | Avg Time per Hash (ns) | Throughput (Mhash/s) |
|---------------|-----------------|----------------|-------------------------|-----------------------|
| **SHA-256**   | 1,000,000,000   | 6.75           | 6.75                   | ~147                 |
| **RIPEMD-160**| 1,000,000,000   | 3.55           | 3.55                   | ~281                 |

---

## 🛠️ **Compile the Code**

To compile the libraries, use the following commands:

```bash
# For SHA-256
g++ -O3 -march=native -mavx2 -fopenmp -std=c++17 sha256.cpp -o sha256

# For RIPEMD-160
g++ -O3 -march=native -mavx2 -fopenmp -std=c++17 ripemd160.cpp -o ripemd160
