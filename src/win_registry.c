#include "general.h"
#include "win_registry.h"
#include <windows.h>
#include <shlwapi.h>

typedef	LONG (WINAPI *RegOpenKeyExPtr)(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
typedef	LONG (WINAPI* RegQueryValueExPtr)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef	LONG (WINAPI* RegGetValuePtr)(HKEY, LPCWSTR, LPCWSTR, DWORD, LPDWORD, PVOID, LPDWORD);
static  RegOpenKeyExPtr pRegOpenKeyExW, sRegOpenKeyExWStub;
static  RegQueryValueExPtr pRegQueryValueExW, sRegQueryValueExWStub;
static  RegGetValuePtr pRegGetValueW, sRegGetValueWStub;

static LONG WINAPI 
HookRegOpenKeyExW(HKEY    hKey,
                  LPCWSTR lpSubKey,
                  DWORD   ulOptions,
                  REGSAM  samDesired,
                  PHKEY   phkResult)
{
    if ( lpSubKey &&
         (StrStrIW(lpSubKey, L"\\Extensions")         || 
         StrStrIW(lpSubKey, L"\\MozillaPlugins")      ||
         //StrStrIW(lpSubKey, L"\\Policies") ||
         StrStrIW(lpSubKey, L"\\QuickTimePlayer.exe") ||
         StrStrIW(lpSubKey, L"\\wmplayer.exe")        ||
         StrStrIW(lpSubKey, L"\\Adobe\\")) )
    {
    #ifdef _LOGDEBUG
        logmsg("RegOpenKeyExW[%ls]\n", lpSubKey);
    #endif
        return ERROR_FILE_NOT_FOUND;
    }
    else if (lpSubKey &&
        (StrStrIW(lpSubKey, L"\\Installer")))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    else if (lpSubKey &&
        (StrStrIW(lpSubKey, L"Mozilla") ||
            StrStrIW(lpSubKey, L"Firefox") ||
            StrStrIW(lpSubKey, L"firefox")))
    {
#ifdef _LOGDEBUG
    //    logmsg("RegOpenKeyExW[%ls]\n", lpSubKey);
#endif
        return ERROR_FILE_NOT_FOUND;
    }
    else
    {
        return sRegOpenKeyExWStub(hKey, lpSubKey, ulOptions, samDesired, phkResult);
    }
}

static LONG WINAPI
HookRegQueryValueExW(HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData)
{
    if (lpValueName &&
        (StrStrIW(lpValueName, L"\\Extensions") ||
            StrStrIW(lpValueName, L"\\MozillaPlugins") ||
            //StrStrIW(lpValueName, L"\\Policies") ||
            StrStrIW(lpValueName, L"\\QuickTimePlayer.exe") ||
            StrStrIW(lpValueName, L"\\wmplayer.exe") ||
            StrStrIW(lpValueName, L"\\Adobe\\")))
    {
#ifdef _LOGDEBUG
   //     logmsg("lpValueName[%ls]\n", lpValueName);
#endif
        return ERROR_FILE_NOT_FOUND;
    }
    else if (lpValueName &&
        (StrStrIW(lpValueName, L"Mozilla") ||
            StrStrIW(lpValueName, L"Firefox") ||
            StrStrIW(lpValueName, L"firefox")))
    {
        return ERROR_FILE_NOT_FOUND;
    }
    else
    {
        return sRegQueryValueExWStub(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
    }
}

static LONG WINAPI
HookRegGetValueW(HKEY    hkey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValue,
    DWORD   dwFlags,
    LPDWORD pdwType,
    PVOID   pvData,
    LPDWORD pcbData)
{
    if (lpSubKey &&
        (StrStrIW(lpSubKey, L"\\Extensions") ||
            StrStrIW(lpSubKey, L"\\MozillaPlugins") ||
            //StrStrIW(lpSubKey, L"\\Policies") ||
            StrStrIW(lpSubKey, L"\\QuickTimePlayer.exe") ||
            StrStrIW(lpSubKey, L"\\wmplayer.exe") ||
            StrStrIW(lpSubKey, L"\\Adobe\\")))
    {
#ifdef _LOGDEBUG
   //     logmsg("RegGetValueW[%ls]\n", lpSubKey);
#endif
        return ERROR_FILE_NOT_FOUND;
    }
    else if ((lpSubKey &&
        (StrStrIW(lpSubKey, L"Mozilla") ||
            StrStrIW(lpSubKey, L"Firefox") ||
            StrStrIW(lpSubKey, L"firefox"))) || (lpValue &&
                (StrStrIW(lpValue, L"Mozilla") ||
                    StrStrIW(lpValue, L"Firefox") ||
                    StrStrIW(lpValue, L"firefox"))))
    {
#ifdef _LOGDEBUG
     //   logmsg("RegGetValueW[%ls]\n", lpSubKey);
#endif
        return ERROR_FILE_NOT_FOUND;
    }
    else
    {
        return sRegGetValueWStub(hkey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }
}

unsigned WINAPI init_winreg(void * pParam)
{
    HMODULE m_adv;
    if ((m_adv = GetModuleHandleW(L"ADVAPI32.dll")) == NULL)
    {
        m_adv = LoadLibrary(L"ADVAPI32.dll");
    }
    if ( m_adv && (pRegOpenKeyExW = (RegOpenKeyExPtr)GetProcAddress(m_adv, "RegOpenKeyExW")) != NULL )
    {
        return creator_hook(pRegOpenKeyExW, HookRegOpenKeyExW, (LPVOID*)&sRegOpenKeyExWStub);
    }
    if (m_adv && (pRegQueryValueExW = (RegQueryValueExPtr)GetProcAddress(m_adv, "RegQueryValueExW")) != NULL)
    {
        return creator_hook(pRegQueryValueExW, HookRegQueryValueExW, (LPVOID*)&sRegQueryValueExWStub);
    }
    if (m_adv && (pRegGetValueW = (RegGetValuePtr)GetProcAddress(m_adv, "RegGetValueW")) != NULL)
    {
        return creator_hook(pRegGetValueW, HookRegGetValueW, (LPVOID*)&sRegGetValueWStub);
    }
    return (0);
}
