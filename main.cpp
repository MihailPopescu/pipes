#include <iostream>
#include <cmath>
#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>

using namespace std;

const int NUM_PROCESSES = 10;
const int NUM_PER_PROCESS = 1000;
const int MAX_NUMBER = 10000;

// Function to check if a number is prime
bool is_prime(int n) {
  if (n <= 1) return false;
  if (n <= 3) return true;

  if (n % 2 == 0 || n % 3 == 0) return false;

  for (int i = 5; i * i <= n; i += 6) {
    if (n % i == 0 || n % (i + 2) == 0) {
      return false;
    }
  }

  return true;
}

int main() {
  // Create pipes for communication between processes
  int pipes[NUM_PROCESSES][2];
  for (int i = 0; i < NUM_PROCESSES; i++) {
    if (pipe(pipes[i]) < 0) {
      cerr << "Error creating pipe" << endl;
      return 1;
    }
  }

  // Create and execute processes
  for (int i = 0; i < NUM_PROCESSES; i++) {
    int start = i * NUM_PER_PROCESS + 1;
    int end = (i + 1) * NUM_PER_PROCESS;

    pid_t pid = fork();
    if (pid < 0) {
      cerr << "Error creating process" << endl;
      return 1;
    } else if (pid == 0) {
      // Child process
      close(pipes[i][0]);  // Close read end of pipe

      // Search for prime numbers in the assigned range
      for (int j = start; j <= end; j++) {
        if (is_prime(j)) {
          write(pipes[i][1], &j, sizeof(j));  // Write to pipe
        }
      }

      close(pipes[i][1]);  // Close write end of pipe
      return 0;
    } else {
      // Parent process
      close(pipes[i][1]);  // Close write end of pipe
    }
  }

  // Collect results from all processes
  vector<int> primes;
  for (int i = 0; i < NUM_PROCESSES; i++) {
    int num;
    while (read(pipes[i][0], &num, sizeof(num)) > 0) {
      primes.push_back(num);
    }

    close(pipes[i][0]);  // Close read end of pipe
  }

  // Wait for all processes to complete
  for (int i = 0; i < NUM_PROCESSES; i++) {
    wait(NULL);
  }

  // Sort and print the results
  sort(primes.begin(), primes.end());
  for (int n : primes) {
    cout << n << endl;
  }

  return 0;
}
