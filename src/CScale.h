#pragma once
#ifndef _CSCALE_H_
#define _CSCALE_H_

// Ĭ��dpi��96, Ҳ����100%����
class CScale
{
    UINT m_dpi;
public:
    CScale() : m_dpi(USER_DEFAULT_SCREEN_DPI)
    {

    }

    static bool SetDPIAware()
    {
        typedef HRESULT(WINAPI* pfn_SetProcessDpiAwareness)(int value);
        typedef HRESULT(WINAPI* pfn_SetProcessDpiAwarenessContext)(int value);
        typedef DPI_AWARENESS_CONTEXT(WINAPI* pfn_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT dpiContext);
        bool isFree = false;
        HMODULE Shcore = GetModuleHandleW(L"Shcore.dll");
        if (!Shcore)
        {
            Shcore = LoadLibraryW(L"Shcore.dll");
            isFree = true;
        }
        if (!Shcore)
            return false;


        // win10��֧�ֵ�����dpi��ʽ
        pfn_SetProcessDpiAwareness pfnSetProcessDpiAwareness = (pfn_SetProcessDpiAwareness)GetProcAddress(Shcore, "SetProcessDpiAwareness");
        pfn_SetProcessDpiAwarenessContext pfnSetProcessDpiAwarenessContext = (pfn_SetProcessDpiAwarenessContext)GetProcAddress(Shcore, "SetProcessDpiAwarenessContext");
        pfn_SetThreadDpiAwarenessContext pfnSetThreadDpiAwarenessContext = (pfn_SetThreadDpiAwarenessContext)GetProcAddress(Shcore, "SetThreadDpiAwarenessContext");
        if (pfnSetProcessDpiAwareness)
        {
            enum PROCESS_DPI_AWARENESS {
                PROCESS_DPI_UNAWARE = 0,    // DPI ��֪���� ��Ӧ�ò������� DPI ���ģ�����ʼ�ռٶ������ϵ��Ϊ 100% (96 DPI) �� ϵͳ�����κ����� DPI �������Զ�������
                PROCESS_SYSTEM_DPI_AWARE = 1,    // ͳ DPI ��֪�� ��Ӧ�ò������� DPI ���ġ� ������ѯ DPI һ�Σ�����Ӧ�õ���������ʹ�ø�ֵ�� ��� DPI �������ģ�Ӧ�ý��������Ϊ�µ� DPI ֵ�� �� DPI ��ϵͳֵ��������ʱ��ϵͳ���Զ�������չ����������
                PROCESS_PER_MONITOR_DPI_AWARE = 2     // �������� DPI ��֪�� ��Ӧ���ڴ��� DPI ʱ��� DPI������ DPI ��������ʱ��������ϵ���� ϵͳ�����Զ�������ЩӦ�ó���
            };

            // ��֪�����Ļ��dpi
            HRESULT hr = pfnSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

            if (pfnSetThreadDpiAwarenessContext)
                pfnSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

            if (FAILED(hr))
            {
                //wstr::dbg(L"����DPI��֪ʧ��, ������ = 0x%08X\n", GetLastError());
#ifdef _DEBUG
                __debugbreak();
#endif
                return false;
            }
        }
        else
        {
            // win10���µ�����dpi��ʽ, ���ܸ�֪�����Ļ
            BOOL bRet = SetProcessDPIAware();
            //if (!bRet)
            //{
            //    wstr::dbg(L"����DPI��֪ʧ��, ������ = 0x%08X\n", GetLastError());
            //    return 0;
            //}
        }

        if (isFree)
            FreeLibrary(Shcore);


        return true;
    }

    // �������ú��dpi, һ���� WM_DPICHANGED ��Ϣ�µ���, ���� LOWORD(wParam)
    inline UINT SetDpi(UINT dpi)
    {
        m_dpi = dpi;
        return m_dpi;
    }
    // �������ú��dpi
    inline UINT SetDpi(HWND hWnd)
    {
        m_dpi = GetDpiForWindow(hWnd);
        return m_dpi;
    }
    inline UINT GetDpi() const
    {
        return m_dpi;
    }
    inline int scale(int x) const
    {
        int ret = MulDiv(x, m_dpi, USER_DEFAULT_SCREEN_DPI);
        if (ret == 0)
            return x;
        return ret;
    }
    inline int scale(RECT& rc) const
    {
        rc.left     = MulDiv(rc.left,   m_dpi, USER_DEFAULT_SCREEN_DPI);
        rc.top      = MulDiv(rc.top,    m_dpi, USER_DEFAULT_SCREEN_DPI);
        rc.right    = MulDiv(rc.right,  m_dpi, USER_DEFAULT_SCREEN_DPI);
        rc.bottom   = MulDiv(rc.bottom, m_dpi, USER_DEFAULT_SCREEN_DPI);
        return rc.left;
    };

    inline int unscale(int x) const
    {
        return MulDiv(x, USER_DEFAULT_SCREEN_DPI, m_dpi);
    };


    inline int rerect(RECT& rc) const
    {
        return unscale(rc);
    }
    inline int unscale(RECT& rc) const
    {
        rc.left     = MulDiv(rc.left    , USER_DEFAULT_SCREEN_DPI, m_dpi);
        rc.top      = MulDiv(rc.top     , USER_DEFAULT_SCREEN_DPI, m_dpi);
        rc.right    = MulDiv(rc.right   , USER_DEFAULT_SCREEN_DPI, m_dpi);
        rc.bottom   = MulDiv(rc.bottom  , USER_DEFAULT_SCREEN_DPI, m_dpi);
        return rc.left;
    };
    
    // �Ѿ��ΰ�ָ��dpiת�����µ�dpiֵ
    // oldDpi��ʾx���ֵԭ����dpiֵ
    // newDpi�������ź��dpiֵ, ���� USER_DEFAULT_SCREEN_DPI ���ǻ�ԭ��100%����
    static int scale(int x, UINT oldDpi, UINT newDpi)
    {
        int ret = MulDiv(x, newDpi, oldDpi);
        if (ret == 0)
            return x;
        return ret;
    }
    
    // �Ѿ��ΰ�ָ��dpiת�����µ�dpiֵ
    // oldDpi��ʾrc�������ԭ����dpiֵ
    // newDpi�������ź��dpiֵ, ���� USER_DEFAULT_SCREEN_DPI ���ǻ�ԭ��100%����
    static int scale(RECT& rc, UINT oldDpi, UINT newDpi)
    {
        rc.left     = scale(rc.left,   oldDpi, newDpi);
        rc.top      = scale(rc.top,    oldDpi, newDpi);
        rc.right    = scale(rc.right,  oldDpi, newDpi);
        rc.bottom   = scale(rc.bottom, oldDpi, newDpi);
        return rc.left;
    };
    // ��ָ��dpi��ֵת�����µ�dpiֵ
    static int unscale(int x, UINT oldDpi, UINT newDpi)
    {
        return MulDiv(x, oldDpi, newDpi);

        // ��ת��Ϊ96������ֵ
        int oldVl = MulDiv(x, USER_DEFAULT_SCREEN_DPI, oldDpi);

        int ret = MulDiv(oldVl, newDpi, USER_DEFAULT_SCREEN_DPI);
        if (ret == 0 || oldVl == 0)
        {
            // ������������
            const double scaleFactor = static_cast<double>(newDpi) / static_cast<double>(oldDpi);
            // Ӧ���������Ӳ�ת��Ϊ����
            return static_cast<int>(static_cast<double>(x) * scaleFactor);
        }
        return ret;
    };

    // ��ָ��dpi��ֵת�����µ�dpiֵ
    static int unscale(RECT& rc, UINT oldDpi, UINT newDpi)
    {
        rc.left     = unscale((int)rc.left  , oldDpi, newDpi);
        rc.top      = unscale((int)rc.top   , oldDpi, newDpi);
        rc.right    = unscale((int)rc.right , oldDpi, newDpi);
        rc.bottom   = unscale((int)rc.bottom, oldDpi, newDpi);
        return rc.left;
    };
public:
    inline int operator()(int x) const
    {
        return scale(x);
    }
    inline int operator()(RECT& rc) const
    {
        return scale(rc);
    }
    inline operator UINT() const
    {
        return m_dpi;
    }
    inline int operator=(UINT dpi)
    {
        return SetDpi(dpi);
    }
    inline int operator=(HWND hWnd)
    {
        return SetDpi(hWnd);
    }
};



#endif  // _CSCALE_H_
