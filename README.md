# DLL-Hijacking: Search Order Hijacking PoC

A Proof-of-Concept (PoC) demonstrating DLL Search Order Hijacking (MITRE ATT&CK [T1574.001](https://attack.mitre.org/techniques/T1574/001/)). This repository contains a deliberately vulnerable host application and a benign malicious DLL that illustrate how Windows resolves unqualified DLL names and how that behavior can be abused.

**Full walkthrough:** [anthonydimayo.com/writing/dll-search-order-hijacking](https://anthonydimayo.com/writing/dll-search-order-hijacking)

---

## Repository Contents

| File | Description |
|------|-------------|
| `vulnerable_app.cpp` | Deliberately vulnerable host application |
| `hijackable.cpp` | Benign malicious DLL --> executes payload via `DllMain` on load |

---

## Code Breakdown

### _vulnerable_app.cpp_

The host application simulates a vulnerable program by calling `LoadLibrary()` with an **unqualified DLL name** — the root cause of this entire attack class.

```cpp
HMODULE hModule = LoadLibrary(L"hijackable.dll");
```

When `LoadLibrary()` receives a bare filename with no full path, Windows searches for the DLL through a predefined sequence of directories called the **DLL search order**:

```
1. Directory the application loaded from
2. System32
3. 16-bit System directory
4. Windows directory
5. Current working directory
6. Directories in PATH
```

Since `hijackable.dll` doesn't exist in System32 or any other trusted location, the search proceeds into writable directories where an attacker can plant a malicious copy. The application loads whatever it finds first, with no way to distinguish the legitimate DLL from the attacker's version.

---

### _hijackable.cpp_

The malicious DLL uses `DllMain()` — the standard Windows DLL entry point — to execute its payload automatically when loaded by the host process. No explicit call is needed; Windows invokes `DllMain()` as part of the loading sequence.

The `ul_reason_for_call` parameter controls when the payload fires. It only executes on `DLL_PROCESS_ATTACH` — the moment the host application first loads the DLL into memory:

```
vulnerable_app.exe starts
         ↓
Windows searches DLL search order
         ↓
hijackable.dll found in writable directory
         ↓
Windows loads DLL → DllMain() called
         ↓
DLL_PROCESS_ATTACH → payload executes
```

**Payload (both actions are benign):**
- Writes a timestamped entry to `hijacked_success.txt` in the working directory — persistent forensic proof of execution
- Displays a MessageBox for immediate visual confirmation

---

## Building

Using VSCode (You will need Visual Studio C++ build tools).

```bat
:: Compile the malicious DLL
cl /LD hijackable.cpp /Fe:hijackable.dll

:: Compile the vulnerable host application
cl vulnerable_app.cpp /Fe:vulnerable_app.exe
```

---

## Running the PoC

1. Place `vulnerable_app.exe` and `hijackable.dll` in the same directory
2. Open a terminal in that directory
3. Run `.\vulnerable_app.exe`
4. Confirm execution:
   - A MessageBox appears
   - `hijacked_success.txt` is created in the working directory with a timestamp

**Optional — observe the search order live:** Open Process Monitor (Sysinternals) before running the app. Filter for `Process Name is vulnerable_app.exe` and `Path contains hijackable`. You will see a sequence of `NAME NOT FOUND` results as Windows walks each directory in the search order, followed by `SUCCESS` when it finds your DLL.

---

## Detection

Sysmon **Event ID 7 (Image Loaded)** captures DLL load events. When this PoC executes, watch for:

- `vulnerable_app.exe` loading `hijackable.dll` from a **writable directory** outside System32
- `hijackable.dll` is **unsigned**: legitimate system DLLs carry a Microsoft signature
- The load path is unexpected for any known legitimate application

For the full detection walkthrough with Sysmon configuration, Wazuh ingestion, and a Sigma rule --> see the [blog post](https://anthonydimayo.com/writing/dll-search-order-hijacking).

---

## References

- MITRE ATT&CK — [T1574.001: DLL Search Order Hijacking](https://attack.mitre.org/techniques/T1574/001/) 
- Microsoft Learn — [DLL Search Order](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order)
- https://www.upguard.com/blog/dll-hijacking 
- Microsoft Learn — [DLL Security](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-security)
- Wietze Beukema — [Hijacking DLLs in Windows](https://www.wietzebeukema.nl/blog/hijacking-dlls-in-windows)
- [hijacklibs.net](https://hijacklibs.net) — Curated database of real-world DLL hijack opportunities
- Red Team Notes — [DLL Search Order Hijacking](https://dmcxblue.gitbook.io/red-team-notes/persistence/dll-search-order-hijacking) 
- Microsoft Learn — [Dynamic Linked Libraries](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-libraries) 
- Palo Alto — https://unit42.paloaltonetworks.com/dll-hijacking-techniques/ 
- https://hijacklibs.net/ 
