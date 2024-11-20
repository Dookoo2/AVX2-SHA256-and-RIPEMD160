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
#include "ripemd160_avx2.h"  // Include the optimized RIPEMD-160 AVX2 header

// Function to increment a byte array by a given value
inline void incrementByteArray(uint8_t* bytes, size_t length, uint64_t increment) {
    for (size_t i = 0; i < length; ++i) {
        uint64_t sum = bytes[i] + (increment & 0xFF);
        bytes[i] = static_cast<uint8_t>(sum);  // Store the least significant byte
        increment = (increment >> 8) + (sum >> 8);  // Carry over to the next byte
        if (increment == 0) break;  // No more increment needed
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
              << "  -i <initial_key>  Specify initial key (64 HEX characters)\n"
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
        {"0000000000000000000000000000000000000000000000000000000000000001", "ae387fcfeb723c3f5964509af111cf5a67f30661"},
        {"0000000000000000000000000000000000000000000000000000000000000010", "89aab4a6e25ba092cc701e6394e975748c195de9"},
        {"0000000000000000000000000000000000000000000000000000000000000100", "63374a012d674b221e1a04b552f0dc433ef68394"},
        {"0000000000000000000000000000000000000000000000000000000000001000", "66afe0607ffc7fd5281d191e81098acf5c738d05"},
        {"1000000000000000000000000000000000000000000000000000000000000000", "7147350ea8019b764d6d235807033fbcddbddb25"},
        {"0100000000000000000000000000000000000000000000000000000000000000", "01b5ab36cb024c433beed66bee022f63af5df20a"},
        {"0010000000000000000000000000000000000000000000000000000000000000", "3ad8e38bf6d0a4d417ab8f91a66f31ab571729ce"},
        {"0001000000000000000000000000000000000000000000000000000000000000", "269d582dffd8b8edb67a298a678aea3d09d27ba6"}
    };

    bool allPassed = true;

    for (const auto& testCase : testCases) {
        uint8_t keyBytes[64] = {0};  // Ensure buffer is at least 64 bytes
        // Convert hex string to byte array
        for (size_t i = 0; i < 32; ++i) {
            std::string byteString = testCase.input.substr(i * 2, 2);
            keyBytes[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        }

        // Prepare input buffers
        unsigned char inputBuffers[8][64] = {0};
        for (int i = 0; i < 8; ++i) {
            memcpy(inputBuffers[i], keyBytes, 32);
        }
        unsigned char* inputs[8];
        for (int i = 0; i < 8; ++i) {
            inputs[i] = inputBuffers[i];
        }

        unsigned char outputs[8][20];

        ripemd160avx2::ripemd160avx2_32(
            inputs[0], inputs[1], inputs[2], inputs[3],
            inputs[4], inputs[5], inputs[6], inputs[7],
            outputs[0], outputs[1], outputs[2], outputs[3],
            outputs[4], outputs[5], outputs[6], outputs[7]
        );

        std::string hashHex = bytesToHexString(outputs[0], 20);
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
    std::string initialKeyHex = "0000000000000000000000000000000000000000000000000000000000011111";  // Default initial key

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
                // Pad with zeros on the left if less than 64 hex digits
                if (initialKeyHex.size() < 64) {
                    initialKeyHex = std::string(64 - initialKeyHex.size(), '0') + initialKeyHex;
                } else if (initialKeyHex.size() > 64) {
                    std::cerr << "Error: Initial key must be at most 64 hex digits.\n";
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
    std::vector<std::vector<unsigned char>> lastHashes(numThreads, std::vector<unsigned char>(20));

    size_t keyLength = 32;  // 32 bytes

    // Convert initial key from hex string to byte array
    uint8_t initialKeyBytes[64] = {0};  // Ensure buffer is at least 64 bytes
    for (size_t i = 0; i < 32; ++i) {
        std::string byteString = initialKeyHex.substr(i * 2, 2);
        initialKeyBytes[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
    }

    #pragma omp parallel
    {
        int threadId = omp_get_thread_num();
        uint64_t hashesPerThread = hashCount / numThreads;

        // Each thread gets a copy of the starting key
        uint8_t startingKeyBytes[64] = {0};
        memcpy(startingKeyBytes, initialKeyBytes, 32);  // Copy only the first 32 bytes

        // Increment starting key for each thread
        incrementByteArray(startingKeyBytes, keyLength, threadId * hashesPerThread);

        unsigned char keysBatch[8][64] = {0};
        unsigned char hashesBatch[8][20];
        unsigned char* keysBatchPtr[8];
        unsigned char* hashesBatchPtr[8];

        for (uint64_t i = 0; i < hashesPerThread; i += 8) {
            // Prepare batch of 8 keys
            for (int j = 0; j < 8; ++j) {
                memcpy(keysBatch[j], startingKeyBytes, keyLength);
                keysBatchPtr[j] = keysBatch[j];
                hashesBatchPtr[j] = hashesBatch[j];
                incrementByteArray(startingKeyBytes, keyLength, 1);
            }

            // Compute hashes using the optimized RIPEMD-160 AVX2 function
            ripemd160avx2::ripemd160avx2_32(
                keysBatchPtr[0], keysBatchPtr[1], keysBatchPtr[2], keysBatchPtr[3],
                keysBatchPtr[4], keysBatchPtr[5], keysBatchPtr[6], keysBatchPtr[7],
                hashesBatchPtr[0], hashesBatchPtr[1], hashesBatchPtr[2], hashesBatchPtr[3],
                hashesBatchPtr[4], hashesBatchPtr[5], hashesBatchPtr[6], hashesBatchPtr[7]
            );

            // Save the last key and hash from this thread
            if (i + 8 >= hashesPerThread) {
                lastKeys[threadId] = bytesToHexString(keysBatch[7], keyLength);
                lastHashes[threadId].assign(hashesBatch[7], hashesBatch[7] + 20);
            }
        }
    }

    auto totalEnd = std::chrono::high_resolution_clock::now();

    // Output last keys and hashes
    if (saveLastHashes) {
        std::ofstream outFile("last_hashes.txt");
        for (int i = 0; i < numThreads; ++i) {
            outFile << "Thread " << i << " last key: " << lastKeys[i] << "\n";
            outFile << "Thread " << i << " last hash: " << bytesToHexString(lastHashes[i].data(), 20) << "\n";
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

