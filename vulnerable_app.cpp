#include <windows.h>
#include <stdio.h>

int main() {
    printf("[*] Attempting to load hijackable.dll...\n");
    printf("[*] This DLL doesn't exist in System32 or other trusted locations.\n");
    printf("[*] Windows will search the search order and may load an attacker's version.\n\n");
    
    HMODULE hModule = LoadLibrary("hijackable.dll");
    
    if (hModule != NULL) {
        printf("[+] Successfully loaded hijackable.dll\n");
        printf("[+] Module base address: 0x%p\n", hModule);
        printf("[+] Check hijacked_success.txt in the current directory for proof.\n");
        FreeLibrary(hModule);
    } else {
        printf("[-] Failed to load hijackable.dll. Error code: %lu\n", GetLastError());
        printf("[-] This is expected if no malicious DLL is present.\n");
    }
    
    printf("\n[*] Press Enter to exit...\n");
    getchar();
    
    return 0;
}
