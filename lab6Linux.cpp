#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool isPrime(int num) {
    if (num < 2) return false;
    for (int i = 2; i <= std::sqrt(num); ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

void findPrimesInRange(int start, int end, int writePipe) {
    std::vector<int> primes;
    for (int i = start; i < end; ++i) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }
    write(writePipe, primes.data(), primes.size() * sizeof(int));
    close(writePipe);
}

int main() {
    const int RANGE = 1000;
    const int TOTAL_PROCESSES = 10;
    int start = 0;
    int end = RANGE;
    int pipes[TOTAL_PROCESSES][2];

    // Create pipes
    for (int i = 0; i < TOTAL_PROCESSES; ++i) {
        if (pipe(pipes[i]) == -1) {
            std::cerr << "Error creating pipe" << std::endl;
            return 1;
        }
    }

    // Fork processes
    for (int i = 0; i < TOTAL_PROCESSES; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Fork failed" << std::endl;
            return 1;
        } else if (pid == 0) {
            // Child process
            close(pipes[i][0]); // Close read end
            findPrimesInRange(start, end, pipes[i][1]);
            return 0;
        } else {
            // Parent process
            close(pipes[i][1]); // Close write end
        }
        start += RANGE;
        end += RANGE;
    }

    // Parent process collects results
    for (int i = 0; i < TOTAL_PROCESSES; ++i) {
        int buffer[RANGE] = {0};
        int bytesRead = read(pipes[i][0], buffer, sizeof(buffer));
        close(pipes[i][0]);

        if (bytesRead > 0) {
            int count = bytesRead / sizeof(int);
            for (int j = 0; j < count; ++j) {
                std::cout << buffer[j] << " ";
            }
        }
        wait(NULL); // Wait for child process
    }

    std::cout << std::endl;
    return 0;
}
