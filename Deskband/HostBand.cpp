#include "stdafx.h"
#include "HostBand.h"

#include <windows.h>
#include <uxtheme.h>
#include <time.h>
#include <cstdio>

#define RECTWIDTH(x)   ((x).right - (x).left)
#define RECTHEIGHT(x)  ((x).bottom - (x).top)

extern ULONG        g_cDllRef;
extern HINSTANCE    g_hInst;

extern CLSID DeskBandCLSID;

static const WCHAR g_szWindowClass[] = L"MonibandDeskband";
static const UINT_PTR g_TickTimerID = 1;

HostBand::HostBand() :
	m_cRef(1), m_pSite(NULL), m_dwBandID(0), m_hwnd(NULL), m_hwndParent(NULL)
{
}

HostBand::~HostBand()
{
	if (m_pSite)
	{
		m_pSite->Release();
	}
}

//
// IUnknown
//
STDMETHODIMP HostBand::QueryInterface(REFIID riid, void **ppv)
{
	HRESULT hr = S_OK;

	if (IsEqualIID(IID_IUnknown, riid) ||
		IsEqualIID(IID_IOleWindow, riid) ||
		IsEqualIID(IID_IDockingWindow, riid) ||
		IsEqualIID(IID_IDeskBand, riid) ||
		IsEqualIID(IID_IDeskBand2, riid))
	{
		*ppv = static_cast<IOleWindow *>(this);
	}
	else if (IsEqualIID(IID_IPersist, riid) ||
		IsEqualIID(IID_IPersistStream, riid))
	{
		*ppv = static_cast<IPersist *>(this);
	}
	else if (IsEqualIID(IID_IObjectWithSite, riid))
	{
		*ppv = static_cast<IObjectWithSite *>(this);
	}
	else if (IsEqualIID(IID_IInputObject, riid))
	{
		*ppv = static_cast<IInputObject *>(this);
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
	}

	if (*ppv)
	{
		AddRef();
	}

	return hr;
}

STDMETHODIMP_(ULONG) HostBand::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) HostBand::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

//
// IOleWindow
//
STDMETHODIMP HostBand::GetWindow(HWND *phwnd)
{
	*phwnd = m_hwnd;
	return S_OK;
}

STDMETHODIMP HostBand::ContextSensitiveHelp(BOOL)
{
	return E_NOTIMPL;
}

//
// IDockingWindow
//
STDMETHODIMP HostBand::ShowDW(BOOL fShow)
{
	if (m_hwnd)
	{
		ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
	}

	return S_OK;
}

STDMETHODIMP HostBand::CloseDW(DWORD)
{
	if (m_hwnd)
	{
		ShowWindow(m_hwnd, SW_HIDE);
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}

	return S_OK;
}

STDMETHODIMP HostBand::ResizeBorderDW(const RECT *, IUnknown *, BOOL)
{
	return E_NOTIMPL;
}

//
// IDeskBand
//
STDMETHODIMP HostBand::GetBandInfo(DWORD dwBandID, DWORD, DESKBANDINFO *pdbi)
{
	HRESULT hr = E_INVALIDARG;

	if (pdbi)
	{
		m_dwBandID = dwBandID;

		if (pdbi->dwMask & DBIM_MINSIZE)
		{
			pdbi->ptMinSize.x = 200;
			pdbi->ptMinSize.y = 30;
		}

		if (pdbi->dwMask & DBIM_MAXSIZE)
		{
			pdbi->ptMaxSize.y = -1;
		}

		if (pdbi->dwMask & DBIM_INTEGRAL)
		{
			pdbi->ptIntegral.y = 1;
		}

		if (pdbi->dwMask & DBIM_ACTUAL)
		{
			pdbi->ptActual.x = 200;
			pdbi->ptActual.y = 30;
		}

		if (pdbi->dwMask & DBIM_TITLE)
		{
			// Don't show title by removing this flag.
			pdbi->dwMask &= ~DBIM_TITLE;
		}

		if (pdbi->dwMask & DBIM_MODEFLAGS)
		{
			pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT;
		}

		if (pdbi->dwMask & DBIM_BKCOLOR)
		{
			// Use the default background color by removing this flag.
			pdbi->dwMask &= ~DBIM_BKCOLOR;
		}

		hr = S_OK;
	}

	return hr;
}

//
// IDeskBand2
//
STDMETHODIMP HostBand::CanRenderComposited(BOOL *pfCanRenderComposited)
{
	*pfCanRenderComposited = TRUE;

	return S_OK;
}

STDMETHODIMP HostBand::SetCompositionState(BOOL fCompositionEnabled)
{
	m_fCompositionEnabled = fCompositionEnabled;

	InvalidateRect(m_hwnd, NULL, TRUE);
	UpdateWindow(m_hwnd);

	return S_OK;
}

STDMETHODIMP HostBand::GetCompositionState(BOOL *pfCompositionEnabled)
{
	*pfCompositionEnabled = m_fCompositionEnabled;

	return S_OK;
}

//
// IPersist
//
STDMETHODIMP HostBand::GetClassID(CLSID *pclsid)
{
	*pclsid = DeskBandCLSID;
	return S_OK;
}

//
// IPersistStream
//
STDMETHODIMP HostBand::IsDirty()
{
	return S_FALSE;
}

STDMETHODIMP HostBand::Load(IStream * /*pStm*/)
{
	return S_OK;
}

STDMETHODIMP HostBand::Save(IStream * /*pStm*/, BOOL /*fClearDirty*/)
{
	return S_OK;
}

STDMETHODIMP HostBand::GetSizeMax(ULARGE_INTEGER * /*pcbSize*/)
{
	return E_NOTIMPL;
}

//
// IObjectWithSite
//
STDMETHODIMP HostBand::SetSite(IUnknown *pUnkSite)
{
	HRESULT hr = S_OK;

	m_hwndParent = NULL;

	if (m_pSite)
	{
		m_pSite->Release();
	}

	if (pUnkSite)
	{
		IOleWindow *pow;
		hr = pUnkSite->QueryInterface(IID_IOleWindow, reinterpret_cast<void **>(&pow));
		if (SUCCEEDED(hr))
		{
			hr = pow->GetWindow(&m_hwndParent);
			if (SUCCEEDED(hr))
			{
				WNDCLASSW wc = { 0 };
				wc.style = CS_HREDRAW | CS_VREDRAW;
				wc.hCursor = LoadCursor(NULL, IDC_ARROW);
				wc.hInstance = g_hInst;
				wc.lpfnWndProc = WndProc;
				wc.lpszClassName = g_szWindowClass;
				wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 0));

				RegisterClassW(&wc);

				CreateWindowExW(0,
					g_szWindowClass,
					NULL,
					WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
					0,
					0,
					0,
					0,
					m_hwndParent,
					NULL,
					g_hInst,
					this);

				if (!m_hwnd)
				{
					hr = E_FAIL;
				}
			}

			pow->Release();
		}

		hr = pUnkSite->QueryInterface(IID_IInputObjectSite, reinterpret_cast<void **>(&m_pSite));
	}

	return hr;
}

STDMETHODIMP HostBand::GetSite(REFIID riid, void **ppv)
{
	HRESULT hr = E_FAIL;

	if (m_pSite)
	{
		hr = m_pSite->QueryInterface(riid, ppv);
	}
	else
	{
		*ppv = NULL;
	}

	return hr;
}

//
// IInputObject
//
STDMETHODIMP HostBand::UIActivateIO(BOOL /*fActivate*/, MSG *)
{
	return S_OK;
}

STDMETHODIMP HostBand::HasFocusIO()
{
	return S_FALSE;
}

STDMETHODIMP HostBand::TranslateAcceleratorIO(MSG *)
{
	return S_FALSE;
};

void HostBand::OnPaint(const HDC hdcIn)
{
	HDC hdc = hdcIn;
	PAINTSTRUCT ps;
	static WCHAR szContent[20];

	time_t rawtime;
	struct tm timeinfo;
	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	int len = swprintf(szContent, 20, L"DeskBand. %d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	if (!hdc)
	{
		hdc = BeginPaint(m_hwnd, &ps);
	}

	if (hdc)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		SIZE size;

		if (m_fCompositionEnabled)
		{
			HTHEME hTheme = OpenThemeData(NULL, L"BUTTON");
			if (hTheme)
			{
				HDC hdcPaint = NULL;
				HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, NULL, &hdcPaint);

				DrawThemeParentBackground(m_hwnd, hdcPaint, &rc);

				GetTextExtentPointW(hdc, szContent, len, &size);
				RECT rcText;
				rcText.left = (RECTWIDTH(rc) - size.cx) / 2;
				rcText.top = (RECTHEIGHT(rc) - size.cy) / 2;
				rcText.right = rcText.left + size.cx;
				rcText.bottom = rcText.top + size.cy;

				DTTOPTS dttOpts = { sizeof(dttOpts) };
				dttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR | DTT_GLOWSIZE;
				dttOpts.crText = RGB(255, 255, 0);
				dttOpts.iGlowSize = 10;
				DrawThemeTextEx(hTheme, hdcPaint, 0, 0, szContent, -1, 0, &rcText, &dttOpts);

				EndBufferedPaint(hBufferedPaint, TRUE);

				CloseThemeData(hTheme);
			}
		}
		else
		{
			SetBkColor(hdc, RGB(255, 255, 0));
			GetTextExtentPointW(hdc, szContent, len, &size);
			TextOutW(hdc,
				(RECTWIDTH(rc) - size.cx) / 2,
				(RECTHEIGHT(rc) - size.cy) / 2,
				szContent,
				ARRAYSIZE(szContent));
		}
	}

	if (!hdcIn)
	{
		EndPaint(m_hwnd, &ps);
	}
}

LRESULT CALLBACK HostBand::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;

	HostBand*pDeskBand = reinterpret_cast<HostBand*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_CREATE:
		pDeskBand = reinterpret_cast<HostBand*>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
		pDeskBand->m_hwnd = hwnd;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDeskBand));
		SetTimer(hwnd, g_TickTimerID, 1000, NULL);
		break;

	case WM_DESTROY:
		KillTimer(hwnd, g_TickTimerID);
		break;

	case WM_PAINT:
		pDeskBand->OnPaint(NULL);
		break;

	case WM_PRINTCLIENT:
		pDeskBand->OnPaint(reinterpret_cast<HDC>(wParam));
		break;

	case WM_ERASEBKGND:
		if (pDeskBand->m_fCompositionEnabled)
		{
			lResult = 1;
		}
		break;

	case WM_TIMER:
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		break;
	}

	if (uMsg != WM_ERASEBKGND)
	{
		lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return lResult;
}
