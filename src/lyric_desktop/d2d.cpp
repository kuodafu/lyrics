#include "d2d.h"
#include <atlbase.h>

KUODAFU_NAMESPACE_BEGIN


static D2D_GDI_DATA_STRUCT d2dInfo;

bool d2d_init(bool isMultiThreaded, bool isDebug)
{
    HRESULT hr = 0;
    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return false;

    hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&d2dInfo.pWICFactory));
    if (FAILED(hr))
        return false;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevels[] =
    {
            D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1
    };

    D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
    if (isDebug)
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

    CComPtr<ID3D11Device> pD3DDevice;    // D3D�豸
    CComPtr<IDXGIDevice1> pDxgiDevice;    // dxgi�豸

    // 1. ����D3D11 �豸, Ҳ���Դ���D3D10, ����D3D11����, ����flag һ��Ҫ�� D3D11_CREATE_DEVICE_BGRA_SUPPORT
    hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
                            featureLevels, _countof(featureLevels),
                            D3D11_SDK_VERSION, &pD3DDevice, nullptr, nullptr);
    if (FAILED(hr))
        return false;

    // 2. ��ȡ IDXGIDevice
    hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDxgiDevice));
    if (FAILED(hr))
        return false;

    // 3. ���� D2D ����
    hr = D2D1CreateFactory(isMultiThreaded? D2D1_FACTORY_TYPE_MULTI_THREADED : D2D1_FACTORY_TYPE_SINGLE_THREADED,
                           __uuidof(ID2D1Factory1), &options, reinterpret_cast<void**>(&d2dInfo.pFactory));
    if (FAILED(hr))
        return false;

    // 4. ����D2D�豸
    hr = d2dInfo.pFactory->CreateDevice(pDxgiDevice, &d2dInfo.pD2DDevice);
    if (FAILED(hr))
        return false;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                __uuidof(IDWriteFactory),
                                reinterpret_cast<IUnknown**>(&d2dInfo.pDWriteFactory));
    if (FAILED(hr))
        return false;

    return true;
}

bool d2d_uninit()
{
    SafeRelease(d2dInfo.pD2DDevice);
    SafeRelease(d2dInfo.pFactory);
    SafeRelease(d2dInfo.pDWriteFactory);
    SafeRelease(d2dInfo.pWICFactory);
    CoUninitialize();
    return true;
}

D2D_GDI_DATA_STRUCT& d2d_get_info()
{
    return d2dInfo;
}



KUODAFU_NAMESPACE_END