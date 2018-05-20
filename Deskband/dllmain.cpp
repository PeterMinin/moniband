// dllmain.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include <windows.h>
#include <strsafe.h> // for StringCchXXX functions
#include <olectl.h> // for SELFREG_E_CLASS
#include <shlobj.h> // for ICatRegister
#include "ClassFactory.h" // for the class factory

// {F30D8CB1-2976-4B0E-813F-AA4C52C7D459}
CLSID DeskBandCLSID = { 0xf30d8cb1, 0x2976, 0x4B0E,{ 0x81, 0x3f, 0xaa, 0x4c, 0x52, 0xc7, 0xd4, 0x59 } };

HINSTANCE   g_hInst = NULL;
long        g_cDllRef = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		g_hInst = hModule;
		DisableThreadLibraryCalls(hModule);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(DeskBandCLSID, rclsid))
	{
		hr = E_OUTOFMEMORY;

		ClassFactory *pClassFactory = new ClassFactory();
		if (pClassFactory)
		{
			hr = pClassFactory->QueryInterface(riid, ppv);
			pClassFactory->Release();
		}
	}

	return hr;
}

STDAPI DllCanUnloadNow()
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}

HRESULT RegisterServer()
{
	WCHAR szCLSID[MAX_PATH];
	StringFromGUID2(DeskBandCLSID, szCLSID, ARRAYSIZE(szCLSID));

	WCHAR szSubkey[MAX_PATH];
	HKEY hKey;

	HRESULT hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
	if (SUCCEEDED(hr))
	{
		hr = E_FAIL;
		if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CLASSES_ROOT,
			szSubkey,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			NULL,
			&hKey,
			NULL))
		{
			WCHAR const szName[] = L"Moniband";
			if (ERROR_SUCCESS == RegSetValueExW(hKey,
				NULL,
				0,
				REG_SZ,
				(LPBYTE)szName,
				sizeof(szName)))
			{
				hr = S_OK;
			}

			RegCloseKey(hKey);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s\\InprocServer32", szCLSID);
		if (SUCCEEDED(hr))
		{
			hr = HRESULT_FROM_WIN32(RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubkey,
				0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL));
			if (SUCCEEDED(hr))
			{
				WCHAR szModule[MAX_PATH];
				if (GetModuleFileNameW(g_hInst, szModule, ARRAYSIZE(szModule)))
				{
					DWORD cch = lstrlen(szModule);
					hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, NULL, 0, REG_SZ, (LPBYTE)szModule, cch * sizeof(szModule[0])));
				}

				if (SUCCEEDED(hr))
				{
					WCHAR const szModel[] = L"Apartment";
					hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (LPBYTE)szModel, sizeof(szModel)));
				}

				RegCloseKey(hKey);
			}
		}
	}

	return hr;
}

HRESULT RegisterComCat()
{
	ICatRegister *pcr;
	HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcr));
	if (SUCCEEDED(hr))
	{
		CATID catid = CATID_DeskBand;
		hr = pcr->RegisterClassImplCategories(DeskBandCLSID, 1, &catid);
		pcr->Release();
	}
	return hr;
}

STDAPI DllRegisterServer()
{
	// Register the deskband object.
	HRESULT hr = RegisterServer();
	if (SUCCEEDED(hr))
	{
		// Register the component category for the deskband object.
		hr = RegisterComCat();
	}

	return SUCCEEDED(hr) ? S_OK : SELFREG_E_CLASS;
}

STDAPI DllUnregisterServer()
{
	WCHAR szCLSID[MAX_PATH];
	StringFromGUID2(DeskBandCLSID, szCLSID, ARRAYSIZE(szCLSID));

	WCHAR szSubkey[MAX_PATH];
	HRESULT hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
	if (SUCCEEDED(hr))
	{
		if (ERROR_SUCCESS != RegDeleteTreeW(HKEY_CLASSES_ROOT, szSubkey))
		{
			hr = E_FAIL;
		}
	}

	return SUCCEEDED(hr) ? S_OK : SELFREG_E_CLASS;
}
