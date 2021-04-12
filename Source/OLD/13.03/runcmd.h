#pragma once
// Slightly modified code from http://ubuntuforums.org/archive/index.php/t-610271.html

// Ancora più interessante e conciso (leggi tutto il thread)

#include <tchar.h>
#include <stdio.h>
#define NOMINMAX
#include <windows.h>
char* output;

char* run_command(LPCTSTR cmd);

int _tmain(int argc, TCHAR* argv[])
{
    if ((output = run_command("cmd.exe /c dir c:\\")))
    {
        MessageBox(0, output, "Success", MB_OK);
        GlobalFree(output);
    }

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

    // Define OUTPUTBUFSIZE to be as big as the largest output you expect
    // from the console app, plus one char. For example, if you
    // expect the app to return 10 chars, define this to be 11.
#define OUTPUTBUFSIZE 4096*10

    const TCHAR ErrorStr[] = "Error";
    const TCHAR NoMem[] = "Out of memory";
    const TCHAR NoPipeMsg[] = "Can't open pipe";
    const TCHAR NoLaunchMsg[] = "Can't start console app";
    const TCHAR NoOutput[] = "Can't read output of console app";

    char* run_command(LPSTR cmd)
    {
        STARTUPINFO sinfo;
        PROCESS_INFORMATION pinfo;
        SECURITY_ATTRIBUTES sattr;
        HANDLE readfh, writefh;
        register char* cbuff;

        // Allocate a buffer to read the app's output
        if (!(cbuff = (char*)GlobalAlloc(GMEM_FIXED, OUTPUTBUFSIZE)))
        {
            MessageBox(0, &NoMem[0], &ErrorStr[0], MB_OK | MB_ICONEXCLAMATION);
            return 0;
        }

        // Initialize the STARTUPINFO struct
        ZeroMemory(&sinfo, sizeof(STARTUPINFO));
        sinfo.cb = sizeof(STARTUPINFO);

        sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

        // Uncomment this if you want to hide the other app's
        // DOS window while it runs
        //    sinfo.wShowWindow = SW_HIDE;

        sinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        sinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        // Initialize security attributes to allow the launched app to
        // inherit the caller's STDOUT, STDIN, and STDERR
        sattr.nLength = sizeof(SECURITY_ATTRIBUTES);
        sattr.lpSecurityDescriptor = 0;
        sattr.bInheritHandle = TRUE;

        // Get a pipe from which we read
        // output from the launched app
        if (!CreatePipe(&readfh, &sinfo.hStdOutput, &sattr, 0))
        {
            // Error opening the pipe
            MessageBox(0, &NoPipeMsg[0], &ErrorStr[0], MB_OK | MB_ICONEXCLAMATION);
            GlobalFree(cbuff);
            return 0;
        }

        // Launch the app. We should return immediately (while the app is running)
        if (!CreateProcess(0, cmd, 0, 0, TRUE, 0, 0, 0, &sinfo, &pinfo))
        {
            MessageBox(0, &NoLaunchMsg[0], &ErrorStr[0], MB_OK | MB_ICONEXCLAMATION);
            CloseHandle(sinfo.hStdInput);
            CloseHandle(writefh);
            CloseHandle(readfh);
            CloseHandle(sinfo.hStdOutput);
            GlobalFree(cbuff);
            return 0;
        }

        // Don't need the read access to these pipes
        CloseHandle(sinfo.hStdInput);
        CloseHandle(sinfo.hStdOutput);

        // We haven't yet read app's output
        sinfo.dwFlags = 0;

        // Input and/or output still needs to be done?
        while (readfh)
        {
            // Capture more output of the app?
            // Read in upto OUTPUTBUFSIZE bytes
            if (!ReadFile(readfh, cbuff + sinfo.dwFlags, OUTPUTBUFSIZE - sinfo.dwFlags, &pinfo.dwProcessId, 0) || !pinfo.dwProcessId)
            {
                // If we aborted for any reason other than that the
                // app has closed that pipe, it's an
                // error. Otherwise, the program has finished its
                // output apparently
                if (GetLastError() != ERROR_BROKEN_PIPE && pinfo.dwProcessId)
                {
                    // An error reading the pipe
                    MessageBox(0, &NoOutput[0], &ErrorStr[0], MB_OK | MB_ICONEXCLAMATION);
                    GlobalFree(cbuff);
                    cbuff = 0;
                    break;
                }

                // Close the pipe
                CloseHandle(readfh);
                readfh = 0;
            }

            sinfo.dwFlags += pinfo.dwProcessId;
        }

        // Close input pipe if it's still open
        if (writefh) CloseHandle(writefh);

        // Close output pipe
        if (readfh) CloseHandle(readfh);

        // Wait for the app to finish
        WaitForSingleObject(pinfo.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);

        // Nul-terminate it
        if (cbuff) *(cbuff + sinfo.dwFlags) = 0;

        // Return the output
        return cbuff;
    }

#ifdef __cplusplus
}
#endif