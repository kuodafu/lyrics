#include "d2d.h"

NAMESPACE_D2D_BEGIN


static D2D_GDI_DATA_STRUCT d2dInfo;

D2D_GDI_DATA_STRUCT::~D2D_GDI_DATA_STRUCT()
{
    SafeRelease(d2dInfo.pGDIInterop);
    SafeRelease(d2dInfo.pD2DDeviceContext);
    SafeRelease(d2dInfo.pD2DDevice);
    SafeRelease(d2dInfo.pDWriteFactory);
    SafeRelease(d2dInfo.pFactory);
    SafeRelease(d2dInfo.pWICFactory);
}

bool d2d_init(bool isDebug)
{
    HRESULT hr = 0;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return false;

    hr = CoCreateInstance(CLSID_WICImagingFactory2,
                          NULL, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&d2dInfo.pWICFactory));
    if (FAILED(hr))
        return false;

    CComPtr<ID3D11Device> pD3DDevice;    // D3D设备
    CComPtr<IDXGIDevice1> pDxgiDevice;    // dxgi设备
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
    };

    D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
    //#ifdef _DEBUG
    //    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    //#else
    //    options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
    //#endif
    if (isDebug)
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;


    // 1. 创建D3D11 设备, 也可以创建D3D10, 不过D3D11更好, 创建flag 一定要有 D3D11_CREATE_DEVICE_BGRA_SUPPORT
    hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags,
                            featureLevels, sizeof(featureLevels) / sizeof(featureLevels[0]),
                            D3D11_SDK_VERSION, &pD3DDevice, 0, 0);
    if (FAILED(hr)) false;

    // 2. 获取 IDXGIDevice
    hr = pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDxgiDevice));
    if (FAILED(hr)) false;

    // 3. 创建 D2D 工厂
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &options, reinterpret_cast<void**>(&d2dInfo.pFactory));
    if (FAILED(hr)) false;

    // 4. 创建D2D设备
    hr = d2dInfo.pFactory->CreateDevice(pDxgiDevice, &d2dInfo.pD2DDevice);
    if (FAILED(hr)) false;

    // 5. 创建D2D设备上下文
    hr = d2dInfo.pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dInfo.pD2DDeviceContext);
    if (FAILED(hr)) false;

    d2dInfo.pD2DDeviceContext->QueryInterface(IID_PPV_ARGS(&d2dInfo.pGDIInterop));
    d2dInfo.pD2DDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
    //d2dInfo.pFactory->GetDesktopDpi(&d2dInfo.DpiX, &d2dInfo.DpiY);
    d2dInfo.pD2DDeviceContext->GetDpi(&d2dInfo.DpiX, &d2dInfo.DpiY);

    d2dInfo.bp_proper.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    d2dInfo.bp_proper.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    d2dInfo.bp_proper.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
    d2dInfo.bp_proper.dpiX = 96;
    d2dInfo.bp_proper.dpiY = 96;
    d2dInfo.bp_proper.colorContext = 0;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                __uuidof(IDWriteFactory),
                                reinterpret_cast<IUnknown**>(&d2dInfo.pDWriteFactory));
    if (FAILED(hr)) false;

    return true;
}

D2D_GDI_DATA_STRUCT& d2d_get_info()
{
    return d2dInfo;
}



NAMESPACE_D2D_END