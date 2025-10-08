<h1>DLL-Hijacking</h1>

"DLL" stands for **Dynamic-Linked-Libraries** which are shared libraries / modules containing code and data that can be used by multiple programs simultaneously. in other words, they're collections of functions, variables, and other resources that applications can load and use during runtime. This is better than having them statically linked into each program's executable file at compile time

**DLL Hijacking** is when malicous code is injected into these DLLs, making it so by replacing a DLL file with an infected version and placing it within the search parameters of an application, the infected file will be called when the application loads

<h2>Detection</h2>

A relevant Sysmon Event ID for detecting DLL-Hijacking would be ```Event Type 7``` which represents “Module/Image Loaded” events. Why? Because DLLs are loaded into the process's memory, which means that any instances of DLLs being loaded into program memory are shown in these events
- <img width="875" height="202" alt="image" src="https://github.com/user-attachments/assets/ab2c7d31-5ea1-4f1f-8c34-561570a6e31d" />

I edited my vms symon config file ```sysmonconfig-export.xml``` so I'm able to see these events in **EventViewer** after editing the Event ID 7 entry like so: 
- <img width="1346" height="152" alt="image" src="https://github.com/user-attachments/assets/335a237f-e307-4eb7-a0ac-408876460431" />


I went to EventViewer : ```Applications and Services Logs → Microsoft → Windows → Sysmon```

TIP → to find DLL files without knowing how it was performed prior, looking for UNSIGNED dll files will uncover the dll responsible 

These logs contain DLL signing status, process or image responsible for loading the DLL, and the specific DLL that was loaded 

Here is a log I looked at:
- <img width="841" height="284" alt="image" src="https://github.com/user-attachments/assets/08277ff7-4759-4491-af75-b442cea1df3a" />

[Here](https://www.wietzebeukema.nl/blog/hijacking-dlls-in-windows) is a list of DLL Hijack techniques that work on Windows 10 (version: 1909) 

Basic rundown of the Hijack I'm investigating:
- Renaming ```reflective_dll.x64.dll``` to ```WININET.dll```
- Moving ```calc.exe``` from ```C:\Windows\System32``` along with ```WININET.dll``` to a writable directory (such as the **Desktop**)
- Executing ```calc.exe```

I applied a filter for **ID 7** and looked for events containing the **calc.exe** string:
- <img width="1246" height="586" alt="image" src="https://github.com/user-attachments/assets/a1586e20-876d-416a-8628-543fcc6b47f3" />

With the filter I was able to find was able to look at indicators of compromise (IOCs):
- “**calc.exe**” is supposed to be found in **system32** or sometimes **Syswow64**, which are NOT writable directories. But, I'm seeing a copy in a writable directory
- "**WININET.dll**", originally located in **System32**, should not be loaded outside of System32 by **calc.exe**. Now that we are seeing a system32 process being loaded in a writable directory by the parent process: **calc.exe** , it’s considered in this case an IOC. HOWEVER, some instances of **WININET.dll** loading outside of system32 are due to certain applications requiring to package certain DLL versions for stability
- The original **WININET.dll** is signed by the OS while the fake one / injected DLL isn’t signed 


