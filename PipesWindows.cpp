#include <iostream>
#include <cmath>
#include <algorithm>
#include <windows.h>
#include <vector>
#include <sstream>

using namespace std;

// Function to check if a number is prime
bool is_prime(int n)
{
    if (n <= 1) return false;
    if (n <= 3) return true;

    if (n % 2 == 0 || n % 3 == 0) return false;

    for (int i = 5; i * i <= n; i += 6)
    {
        if (n % i == 0 || n % (i + 2) == 0)
        {
            return false;
        }
    }

    return true;
}

// Function to create a child process
HANDLE CreateChildProcess(HANDLE hReadPipe, HANDLE hWritePipe)
{
    // Set up the child process's command line arguments
    char* cmdline[] = { "", "0", "1000" };  // modify as needed
    int cmdline_size = sizeof(cmdline) / sizeof(cmdline[0]);

    // Set up the child process's startup info
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;
    si.hStdInput = hReadPipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcess(NULL,cmdline[0],NULL,NULL,TRUE,0,NULL,NULL,&si,&pi)
       )
    {
        cerr << "Error creating child process" << endl;
        return NULL;
    }

    // Close handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return pi.hProcess;
}

int main(int argc, char* argv[])
{
    // Check if this is the child process
    if (argc == 3)
    {
        // Parse start and end values from command line arguments
        int start, end;
        stringstream(argv[1]) >> start;
        stringstream(argv[2]) >> end;

        // Find prime numbers in the given range
        vector<int> primes;
        for (int i = start; i <= end; i++)
        {
            if (is_prime(i))
            {
                primes.push_back(i);
            }
        }

        // Write the results to the pipe
        DWORD bytes_written;
        for (int n : primes)
        {
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), &n, sizeof(n), &bytes_written, NULL);
        }

        return 0;
    }
    else
    {
        const int NUM_PROCESSES = 10;
        const int NUM_PER_PROCESS = 1000;
        const int MAX_NUMBER = 10000;

        // Create pipes for communication between processes
        HANDLE pipes[NUM_PROCESSES][2];
        for (int i = 0; i < NUM_PROCESSES; i++)
        {
            if (!CreatePipe(&pipes[i][0], &pipes[i][1], NULL, 0))
            {
                cerr << "Error creating pipe" << endl;
                return 1;
            }
        }

        // Create child processes
        vector<HANDLE> child_processes;
        for (int i = 0; i < NUM_PROCESSES; i++)
        {
            // Create a child process
            HANDLE child_process = CreateChildProcess(pipes[i][
                                       0], pipes[i][1]);
            if (child_process == NULL)
            {
                cerr << "Error creating child process" << endl;
                return 1;
            }

            child_processes.push_back(child_process);

            // Close the write end of the pipe in the main process
            CloseHandle(pipes[i][1]);
        }

        // Wait for child processes to complete
        WaitForMultipleObjects((DWORD)child_processes.size(), child_processes.data(), TRUE, INFINITE);

        // Collect results from all processes
        vector<int> primes;
        for (int i = 0; i < NUM_PROCESSES; i++)
        {
            int num;
            DWORD bytes_read;

            while (ReadFile(pipes[i][0], &num, sizeof(num), &bytes_read, NULL) && bytes_read > 0)
            {
                primes.push_back(num);
            }
            CloseHandle(pipes[i][0]);  // Close read end of pipe
        }

        // Sort and print the results
        sort(primes.begin(), primes.end());
        for (int n : primes)
        {
            cout << n << endl;
        }

        return 0;
    }
}

