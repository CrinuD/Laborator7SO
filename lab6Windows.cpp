#include <iostream>
#include <vector>
#include <windows.h>

bool checkPrime(int number) {
    if (number < 2) return false;
    for (int div = 2; div * div <= number; ++div) {
        if (number % div == 0) return false;
    }
    return true;
}

DWORD WINAPI processPrimes(LPVOID args) {
    int* limits = (int*)args;
    int begin = limits[0];
    int finish = limits[1];
    HANDLE writePipe = (HANDLE)limits[2];
    std::vector<int> primeNumbers;

    for (int num = begin; num <= finish; ++num) {
        if (checkPrime(num)) {
            primeNumbers.push_back(num);
        }
    }

    DWORD bytesWritten;
    WriteFile(writePipe, primeNumbers.data(), primeNumbers.size() * sizeof(int), &bytesWritten, NULL);
    CloseHandle(writePipe);
    return 0;
}

int main() {
    const int numThreads = 10;
    const int intervalSize = 1000;
    HANDLE pipes[numThreads][2];
    HANDLE threads[numThreads];
    int intervals[numThreads][3];

    for (int i = 0; i < numThreads; ++i) {
        int lowerBound = i * intervalSize + 1;
        int upperBound = (i + 1) * intervalSize;
        CreatePipe(&pipes[i][0], &pipes[i][1], NULL, 0);
        intervals[i][0] = lowerBound;
        intervals[i][1] = upperBound;
        intervals[i][2] = (int)pipes[i][1];
        threads[i] = CreateThread(NULL, 0, processPrimes, intervals[i], 0, NULL);
    }

    for (int i = 0; i < numThreads; ++i) {
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
    }

    for (int i = 0; i < numThreads; ++i) {
        std::vector<int> primeList(intervalSize);
        DWORD bytesRead;
        ReadFile(pipes[i][0], primeList.data(), intervalSize * sizeof(int), &bytesRead, NULL);
        for (int prime : primeList) {
            if (prime != 0) {
                std::cout << prime << " ";
            }
        }
        CloseHandle(pipes[i][0]);
    }

    return 0;
}
