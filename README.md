<h1>Code Breakdown</h1>

<h2>Overview</h2>

This Proof-of-Concept (PoC) DLL demonstrates the execution phase of a DLL Search Order Hijacking attack. Instead of performing any malicious actions, the DLL creates a timestamped marker file and displays a message box when loaded. This cleanly demonstrates that the DLL was successfully loaded and executed by the target application.

<h2>Required Header Files</h2>

```
#include <windows.h>
#include <stdio.h>
#include <time.h>
```
The code begins by importing three header files. The **windows.h** header provides access to the Windows API, which is used for functions such as `MessageBoxA()` and the DLL entry point definition `DllMain()` (this function is being called to handle initialization / when the process loads the DLL). 

The **stdio.h** header supplies standard file input/output operations, which will be used to create and write to the marker file. 

Finally, **time.h** provides date/time functions used to generate timestamps that are written to the log file each time the DLL executes.

<h2>DLL Entry-Point</h2>

```
BOOL APIENTRY DllMain(
  HMODULE hModule,
  DWORD ul_reason_for_call,
  LPVOID lpReserved )
```
Every Windows DLL contains an entry point known as `DllMain()`. This function serves a similar purpose to the `main()` function in a traditional executable. Whenever Windows loads or unloads a DLL, it automatically invokes this function.

The **hModule** parameter contains a handle to the DLL itself. A "handle" is basically a way that windows gives you a reference to an object, which in this case would be the DLL. Using this also allows you to see where the DLL is loaded (memory address), what it's filename is and what resources are embedded in it. However, for this demo that info is ignored. **lpReserved** is a windows-managed pointer that gives extra context regarding how the DLL is being loaded or unloaded.

**ul_reason_for_call** indicates why the function was called. This parameter specifies why Windows invoked `DllMain()`, such as when a process loads the DLL, unloads the DLL, creates a thread, or destroys a thread. It's critical becuase it allows the DLL to determine when the target process has loaded the library and when the demonstration payload should execute

For this PoC, the most important parameter is **ul_reason_for_call**, as it tells the DLL whether it is being loaded into a process, unloaded from a process, attached to a thread, or detached from a thread.

<h2>Determining Why the DLL Was Loaded</h2>

```switch (ul_reason_for_call) {```

The switch statement allows the program to comapre the value of **ul_reason_for_call** against several possible values and then execte the corresponding code block. This switch statement allows the DLL to react differently depending on the reason. 

```
ul_reason_for_call
          ↓
       switch
          ↓
 ┌────────┼────────┐
 ↓        ↓        ↓
PROCESS  THREAD  DETACH
ATTACH   ATTACH
```

In our situation, the code is wating for a process to be attatched before executing the payload. 

When windows loads a DLL into a process for the first time, it generates a **DLL_PROCESS_ATTACH** event. In a Search Order Hijack, this is the moment when the vulnerable application loads the attacker's DLL instead of the legitimate library:

```
VictimApp.exe starts
        ↓
VictimApp.exe requires Example.dll
        ↓
Windows searches for Example.dll
        ↓
Attacker-controlled DLL is found
        ↓
Windows loads DLL
        ↓
DllMain()
        ↓
DLL_PROCESS_ATTACH
```

Once the DLL_PROCESS_ATTATCH event occurs, the payload runs. This is one of the reasons DLL hijacking can be dangerous. The vulnerable application unintentionally executes code simply by loading what it believes is a legitimate dependency.

<h2>Creating the Marker File</h2>

```
FILE* pFile;
errno_t err = fopen_s(&pFile, "hijacked_success.txt", "a");
```

**pFile** is a pointer to a file object, which will allow `fprintf()` and `fclose()` to know which file they should operate on.

The first action performed by the payload is opening a file named _hijacked_success.txt_. which will be created within the applications working directory. notice the 3rd parameter is **"a"** (append) which means that multiple successful executions can be logged in the file. If the file doesn't already exist, it's automatically created. If it already exists, new entries are appended to the end of the file rather than overwriting previous data.

<h2>Error Checking</h2>

```
if (err == 0 && pFile) {
```

Before writing to the file, the code verifies that the file was opened successfully.

The condition checks two things:
- `err == 0` confirms that no error occurred during file creation.
- **pFile** confirms that the file pointer is valid and not NULL.

Performing this validation helps prevent crashes or unexpected behavior if the file cannot be created. This sort of defensive programming isn't necessary but was done for the sake of developing good habits.

If both conditions are true, then the DLL proceeds with logging.

<h2>Generating a Timestamp</h2>

```
time_t now = time(NULL);
```

The `time()` function retrieves the current system time and stores it in the variable now.

This value is represented internally as the number of seconds that have elapsed since January 1, 1970 (Unix Epoch Time).
- Example) `January 1, 1970 00:00:00 UTC = 1749051042`

To make the timestamp readable, the code later converts it into a formatted string.

<h2>Writing Evidence of Execution</h2>

```
fprintf(
  pFile,
  "[%s] DLL hijacking successful - malicious DLL executed via DllMain\n",
  ctime(&now)
);
```

The `fprintf()` function writes a log entry to the marker file (**pFile**). 

The timestamp generated by `time()` is converted into a readable string using `ctime()`. The resulting output resembles:
```
**[Thu Jun 4 15:30:42 2026]
DLL hijacking successful - malicious DLL executed via DllMain**
```

This entry serves as persistent proof that the DLL was loaded and executed by the target process.

<h2>Closing the File</h2>

```
fclose(pFile);
```

After writing the log entry, the file is closed. Closing the file ensures that all buffered data is written to disk and releases the associated system resources. This is a standard best practice whenever file operations are performed.

<h2>Confirmation Message</h2>

```
MessageBoxA(
  NULL,
  "DLL Hijacking PoC: Malicious DLL executed!\n\nCheck hijacked_success.txt for proof.",
  "DLL Search-Order Hijacking Demonstration",
  MB_OK | MB_ICONINFORMATION
);
```

After writing the marker file, the DLL displays a Windows message box. This popup is meant to give immediate visual confirmation that the DLL was successfully loaded and executed. The message also tells the user to check the generated text file for additional proof of execution.

The flags used within the function specify how the dialog should appear:

- **MB_OK** creates a standard OK button.
- **MB_ICONINFORMATION** displays the informational icon.

<h2>Ignored DLL Events</h2>

```
case DLL_PROCESS_DETACH:
case DLL_THREAD_ATTACH:
case DLL_THREAD_DETACH:
break;
```

Windows may call `DllMain()` for several additional events like thread creation/termination or process shutdown.

These events are intentionally ignored because they aren't relevant to demonstrating DLL Search Order Hijacking. The PoC only needs to execute when the DLL is initially loaded into memory.

<h2>Execution Flow</h2>

The complete execution sequence is as follows:

```
Application Starts
        ↓
Windows Searches for Required DLL
        ↓
Attacker-Controlled DLL Found First
        ↓
Windows Loads DLL
        ↓
DllMain() Executes
        ↓
Marker File Created
        ↓
Message Box Displayed
```

This flow demonstrates the core principle behind DLL Search Order Hijacking: if an attacker-controlled DLL is loaded before the legitimate DLL, arbitrary code within the malicious DLL will execute in the context of the target process.
