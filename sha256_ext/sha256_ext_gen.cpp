#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include <omp.h>
#include <immintrin.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include "sha256_ext.h"

// Function to increment a byte array by a given value
inline void incrementByteArray(uint8_t* bytes, size_t length, uint64_t increment) {
    for (size_t i = length; i-- > 0;) {
        uint64_t sum = bytes[i] + (increment & 0xFF);
        bytes[i] = static_cast<uint8_t>(sum);
        increment = (increment >> 8) + (sum >> 8);
    }
}

// Function to convert a byte array to a hexadecimal string
std::string bytesToHexString(const uint8_t* bytes, size_t length) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

// Function to display help message
void displayHelp() {
    std::cout << "Usage: program [options]\n"
              << "Options:\n"
              << "  -h                Display help information\n"
              << "  -c <count>        Number of hashes to compute (multiple of 8, default 128)\n"
              << "  -t <threads>      Number of threads to use (default is maximum available)\n"
              << "  -s                Save last keys and hashes from each thread to last_hashes.txt\n"
              << "  -i <initial_key>  Specify initial key (66 HEX characters)\n"
              << "  --test            Run test cases with known examples\n";
}

// Known test cases for --test option
struct TestCase {
    std::string input;
    std::string expectedHash;
};

// Function to run test cases
bool runTests() {
    std::vector<TestCase> testCases = {
        {"000000000000000000000000000000000000000000000000000000000000000001", "1fd4247443c9440cb3c48c28851937196bc156032d70a96c98e127ecb347e45f"},
        {"000000000000000000000000000000000000000000000000000000000000000010", "65074aa3a766c6973ac3de47f82f3acabb999b36202ce6edcd6b348ea227c00f"},
        {"000000000000000000000000000000000000000000000000000000000000000100", "e65b66499024a7a2f732fc71a035ef70b6eb493445a6cfb708f69bff18fd7eb5"},
        {"000000000000000000000000000000000000000000000000000000000000001000", "58ba123afbfa01d0ab579910e3c71707804db829410c38d132d9f9ceaccc455f"},
        {"100000000000000000000000000000000000000000000000000000000000000000", "a9deba97c5a6ecfff3bd534250e4d43e44732733254e794ca53727344f5522eb"},
        {"010000000000000000000000000000000000000000000000000000000000000000", "1a7dfdeaffeedac489287e85be5e9c049a2ff6470f55cf30260f55395ac1b159"},
        {"001000000000000000000000000000000000000000000000000000000000000000", "cd6a3ca0edecc51cce376f20c24705f61767c0724ab5dc6ba9f97476cf913231"},
        {"000100000000000000000000000000000000000000000000000000000000000000", "1b7c643b049c11a4fb6b65822e4a5e2a3da54223366c5f453cf376536f9bf42a"}
    };

    bool allPassed = true;

    for (const auto& testCase : testCases) {
        uint8_t keyBytes[66] = {0};
        // Convert hex string to byte array
        std::string initialKeyHex = testCase.input;
        if (initialKeyHex.size() < 66) {
            initialKeyHex = std::string(66 - initialKeyHex.size(), '0') + initialKeyHex;
        }

        for (size_t i = 0; i < 33; ++i) {
            std::string byteString = initialKeyHex.substr(i * 2, 2);
            keyBytes[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        }

        // Prepare input buffers
        alignas(32) uint8_t inputBuffers[8][64] = {0};
        for (int i = 0; i < 8; ++i) {
            memcpy(inputBuffers[i], keyBytes, 33);
            inputBuffers[i][33] = 0x80;
            uint64_t bitLength = 33 * 8;
            bitLength = __builtin_bswap64(bitLength);
            memcpy(inputBuffers[i] + 56, &bitLength, 8);
        }
        const uint8_t* inputs[8];
        for (int i = 0; i < 8; ++i) {
            inputs[i] = inputBuffers[i];
        }

        alignas(32) unsigned char outputs[8][32];

        sha256sha_8B(
            inputs[0], inputs[1], inputs[2], inputs[3],
            inputs[4], inputs[5], inputs[6], inputs[7],
            outputs[0], outputs[1], outputs[2], outputs[3],
            outputs[4], outputs[5], outputs[6], outputs[7]
        );

        std::string hashHex = bytesToHexString(outputs[0], 32);
        if (hashHex != testCase.expectedHash) {
            std::cout << "Test failed for input: " << testCase.input << "\n"
                      << "Expected: " << testCase.expectedHash << "\n"
                      << "Got:      " << hashHex << "\n";
            allPassed = false;
        } else {
            std::cout << "Test passed for input: " << testCase.input << "\n";
        }
    }

    return allPassed;
}

int main(int argc, char* argv[]) {
    uint64_t hashCount = 128;  // Default number of hashes
    int numThreads = omp_get_max_threads();  // Default number of threads
    bool saveLastHashes = false;
    bool testMode = false;
    std::string initialKeyHex = "000000000000000000000000000000000000000000000000000000000000011111";  // Default initial key

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h") {
            displayHelp();
            return 0;
        } else if (arg == "-c") {
            if (i + 1 < argc) {
                try {
                    hashCount = std::stoull(argv[++i]);
                    if (hashCount == 0 || hashCount % 8 != 0) {
                        std::cerr << "Error: -c value must be a positive multiple of 8.\n";
                        return 1;
                    }
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: Invalid value for -c.\n";
                    return 1;
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: Value for -c is out of range.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: -c requires a value.\n";
                return 1;
            }
        } else if (arg == "-t") {
            if (i + 1 < argc) {
                try {
                    numThreads = std::stoi(argv[++i]);
                    if (numThreads <= 0) {
                        std::cerr << "Error: -t value must be a positive integer.\n";
                        return 1;
                    }
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: Invalid value for -t.\n";
                    return 1;
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: Value for -t is out of range.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: -t requires a value.\n";
                return 1;
            }
        } else if (arg == "-s") {
            saveLastHashes = true;
        } else if (arg == "-i") {
            if (i + 1 < argc) {
                initialKeyHex = argv[++i];
                // Pad with zeros on the left if less than 66 hex digits
                if (initialKeyHex.size() < 66) {
                    initialKeyHex = std::string(66 - initialKeyHex.size(), '0') + initialKeyHex;
                } else if (initialKeyHex.size() > 66) {
                    std::cerr << "Error: Initial key must be at most 66 hex digits.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: -i requires a value.\n";
                return 1;
            }
        } else if (arg == "--test") {
            testMode = true;
            if (argc > 2) {
                std::cerr << "Error: --test cannot be used with other options.\n";
                return 1;
            }
            break;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            displayHelp();
            return 1;
        }
    }

    if (testMode) {
        bool testsPassed = runTests();
        return testsPassed ? 0 : 1;
    }

    // Check if hashCount is divisible by numThreads
    if (hashCount % numThreads != 0) {
        std::cerr << "Error: Number of hashes must be divisible by the number of threads.\n";
        return 1;
    }

    omp_set_num_threads(numThreads);

    std::cout << "Number of threads                  : " << numThreads << "\n";

    auto totalStart = std::chrono::high_resolution_clock::now();

    // Vectors to store last keys and hashes from each thread
    std::vector<std::string> lastKeys(numThreads);
    std::vector<std::vector<unsigned char>> lastHashes(numThreads, std::vector<unsigned char>(32));

    size_t keyLength = 33;  // 33 bytes

    // Convert initial key from hex string to byte array
    uint8_t initialKeyBytes[66] = {0};
    for (size_t i = 0; i < 33; ++i) {
        std::string byteString = initialKeyHex.substr(i * 2, 2);
        initialKeyBytes[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
    }

    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        uint64_t hashesPerThread = hashCount / numThreads;

        // Each thread gets a copy of the starting key
        uint8_t startingKeyBytes[66] = {0};
        memcpy(startingKeyBytes, initialKeyBytes, keyLength);

        // Increment starting key for each thread
        incrementByteArray(startingKeyBytes, keyLength, threadId * hashesPerThread);

        alignas(32) unsigned char hash[8][32];  // Buffers for hashes
        alignas(32) uint8_t keys[8][64];        // Buffers for keys and padding

        const int batchSize = 8;

        for (uint64_t i = 0; i < hashesPerThread; i += batchSize) {  // Process hashes in batches of 8
            // Initialize input data
            const uint8_t* keysBatch[8];
            unsigned char* hashesBatch[8];

            for (int j = 0; j < batchSize; ++j) {
                memset(keys[j], 0, 64);
                memcpy(keys[j], startingKeyBytes, keyLength);
                keys[j][keyLength] = 0x80;

                uint64_t bitLength = keyLength * 8;
                bitLength = __builtin_bswap64(bitLength);
                memcpy(keys[j] + 56, &bitLength, 8);

                keysBatch[j] = keys[j];
                hashesBatch[j] = hash[j];

                // Increment key for next value
                incrementByteArray(startingKeyBytes, keyLength, 1);
            }

            // Compute hashes using the optimized SHA-256 AVX2 function
            sha256sha_8B(
                keysBatch[0], keysBatch[1], keysBatch[2], keysBatch[3],
                keysBatch[4], keysBatch[5], keysBatch[6], keysBatch[7],
                hashesBatch[0], hashesBatch[1], hashesBatch[2], hashesBatch[3],
                hashesBatch[4], hashesBatch[5], hashesBatch[6], hashesBatch[7]
            );

            // Save the last key and hash from this thread
            if (i + batchSize >= hashesPerThread) {
                lastKeys[threadId] = bytesToHexString(keys[batchSize - 1], keyLength);
                lastHashes[threadId].assign(hash[batchSize - 1], hash[batchSize - 1] + 32);
            }
        }
    }

    auto totalEnd = std::chrono::high_resolution_clock::now();

    // Output last keys and hashes
    if (saveLastHashes) {
        std::ofstream outFile("last_hashes.txt");
        for (int i = 0; i < numThreads; ++i) {
            outFile << "Thread " << i << " last key: " << lastKeys[i] << "\n";
            outFile << "Thread " << i << " last hash: " << bytesToHexString(lastHashes[i].data(), 32) << "\n";
        }
        outFile.close();
    }

    // Output statistics
    auto totalDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(totalEnd - totalStart).count();
    double totalSeconds = totalDuration / 1e9;
    double avgHashTime = (totalDuration / static_cast<double>(hashCount));

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total execution time      (seconds): " << totalSeconds << "\n";
    std::cout << "Average time per hash (nanoseconds): " << avgHashTime << "\n";

    return 0;
}

