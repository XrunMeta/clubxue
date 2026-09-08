

#include "StreamlineDXGISwapchainProxy.h"
#include "sl.h"
#include "StreamlineRHI.h"

#include "Misc/EngineVersionComparison.h"

DEFINE_LOG_CATEGORY(LogStreamlineDXGIRHI);

FStreamlineDXGISwapChainProxy::FStreamlineDXGISwapChainProxy(IDXGISwapChain* InOriginal, const FStreamlineRHI* InStreamlineRHI, bool bInIsStreamlineProxy)
	: WrappedSwapchain(InOriginal)
	, RefCount(1)
	, StreamlineRHI(InStreamlineRHI)
	, bIsStreamlineProxy(bInIsStreamlineProxy)
{
	UE_LOG(LogStreamlineDXGIRHI, VeryVerbose, TEXT("%s %s Enter 0x%p wrapping 0x%p bIsStreamlineProxy=%u"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, WrappedSwapchain, bIsStreamlineProxy);
	check(WrappedSwapchain != nullptr);
	check(StreamlineRHI);

	WrappedSwapchain->AddRef();

	UE_LOG(LogStreamlineDXGIRHI, VeryVerbose, TEXT("%s %s Exit"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__));
}

bool FStreamlineDXGISwapChainProxy::IsSwapChainInterface(REFIID riid)
{
	return riid == __uuidof(IDXGISwapChain)
		|| riid == __uuidof(IDXGISwapChain1)
		|| riid == __uuidof(IDXGISwapChain2)
		|| riid == __uuidof(IDXGISwapChain3)
		|| riid == __uuidof(IDXGISwapChain4);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::QueryInterface(REFIID riid, void** ppvObj)
{
	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Enter 0x%p refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, (LONG)RefCount);

	if (ppvObj == nullptr)
	{
		UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p -> E_POINTER refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, (LONG)RefCount);
		return E_POINTER;
	}

	*ppvObj = nullptr;

	if (IsSwapChainInterface(riid))
	{

		HRESULT hr = WrappedSwapchain->QueryInterface(riid, ppvObj);
		if (FAILED(hr))
		{
			UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p -> 0x%08x (native unsupported) refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, hr, (LONG)RefCount);
			return hr;
		}

		static_cast<IUnknown*>(*ppvObj)->Release();
	}

	if (IsSwapChainInterface(riid) || (riid == __uuidof(FStreamlineDXGISwapChainProxy)))
	{
		AddRef();
		*ppvObj = this;
		UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p -> S_OK (proxy) refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, (LONG)RefCount);
		return S_OK;
	}

	HRESULT hr = WrappedSwapchain->QueryInterface(riid, ppvObj);
	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p -> 0x%08x (passthrough) refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, hr, (LONG)RefCount);
	return hr;
}

ULONG STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::AddRef()
{
	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Enter 0x%p refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, (LONG)RefCount);
	const LONG Ref = ++RefCount;
	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, Ref);
	return Ref;
}

ULONG STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::Release()
{
	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Enter 0x%p refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, (LONG)RefCount);
	const LONG Ref = --RefCount;
	if (Ref > 0)
	{
		UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p refcount=%ld"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, Ref);
		return Ref;
	}

	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s final release 0x%p refcount=%ld bIsStreamlineProxy=%u"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, Ref, bIsStreamlineProxy);

	StreamlineRHI->OnSwapchainDestroyed(this,  bIsStreamlineProxy);
	const ULONG WrappedRefCount = WrappedSwapchain->Release();
	UE_LOG(LogStreamlineDXGIRHI, Verbose, TEXT("%s %s Exit 0x%p - wrapped swapchain 0x%p ref count %lu bIsStreamlineProxy=%u"), *CurrentThreadName(), ANSI_TO_TCHAR(__FUNCTION__), this, WrappedSwapchain, WrappedRefCount, bIsStreamlineProxy);

	delete this;
	return 0;
}

bool FStreamlineDXGISwapChainProxy::IsEnabled()
{
#if UE_VERSION_OLDER_THAN(5,8,0)
	return !ShouldUseSlateCallbacksForSwapchainTracking();
#else
	return true;
#endif
}

bool FStreamlineDXGISwapChainProxy::WrapSwapChain(IDXGISwapChain** ppSwapChain, const FStreamlineRHI* InStreamlineRHI, bool bIsStreamlineProxy)
{
	check(ppSwapChain && *ppSwapChain);

	if (ppSwapChain == nullptr || *ppSwapChain == nullptr)
	{
		return false;
	}

	FStreamlineDXGISwapChainProxy* Proxy = new FStreamlineDXGISwapChainProxy(*ppSwapChain, InStreamlineRHI, bIsStreamlineProxy);

	(*ppSwapChain)->Release();
	*ppSwapChain = Proxy;

	return true;
}

bool FStreamlineDXGISwapChainProxy::WrapSwapChain(IDXGISwapChain1** ppSwapChain, const FStreamlineRHI* InStreamlineRHI, bool bIsStreamlineProxy)
{
	return WrapSwapChain(reinterpret_cast<IDXGISwapChain**>(ppSwapChain), InStreamlineRHI, bIsStreamlineProxy);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::Present(UINT SyncInterval, UINT Flags)
{
	return WrappedSwapchain->Present(SyncInterval, Flags);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::Present1(UINT SyncInterval, UINT PresentFlags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->Present1(SyncInterval, PresentFlags, pPresentParameters);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetPrivateData(REFGUID Name, UINT DataSize, const void* pData)
{
	return WrappedSwapchain->SetPrivateData(Name, DataSize, pData);
}
HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown)
{
	return WrappedSwapchain->SetPrivateDataInterface(Name, pUnknown);
}
HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData)
{
	return WrappedSwapchain->GetPrivateData(Name, pDataSize, pData);
}
HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetParent(REFIID riid, void** ppParent)
{
	return WrappedSwapchain->GetParent(riid, ppParent);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetDevice(REFIID riid, void** ppDevice)
{
	return WrappedSwapchain->GetDevice(riid, ppDevice);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
{
	return WrappedSwapchain->GetBuffer(Buffer, riid, ppSurface);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget)
{
	return WrappedSwapchain->SetFullscreenState(Fullscreen, pTarget);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget)
{
	return WrappedSwapchain->GetFullscreenState(pFullscreen, ppTarget);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc)
{
	return WrappedSwapchain->GetDesc(pDesc);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	return WrappedSwapchain->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters)
{
	return WrappedSwapchain->ResizeTarget(pNewTargetParameters);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetContainingOutput(IDXGIOutput** ppOutput)
{
	return WrappedSwapchain->GetContainingOutput(ppOutput);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats)
{
	return WrappedSwapchain->GetFrameStatistics(pStats);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetLastPresentCount(UINT* pLastPresentCount)
{
	return WrappedSwapchain->GetLastPresentCount(pLastPresentCount);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetDesc1(pDesc);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetFullscreenDesc(pDesc);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetHwnd(HWND* pHwnd)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetHwnd(pHwnd);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetCoreWindow(REFIID refiid, void** ppUnk)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetCoreWindow(refiid, ppUnk);
}

BOOL STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::IsTemporaryMonoSupported()
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->IsTemporaryMonoSupported();
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetRestrictToOutput(IDXGIOutput** ppRestrictToOutput)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetRestrictToOutput(ppRestrictToOutput);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetBackgroundColor(const DXGI_RGBA* pColor)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->SetBackgroundColor(pColor);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetBackgroundColor(DXGI_RGBA* pColor)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetBackgroundColor(pColor);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetRotation(DXGI_MODE_ROTATION Rotation)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->SetRotation(Rotation);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetRotation(DXGI_MODE_ROTATION* pRotation)
{
	return static_cast<IDXGISwapChain1*>(WrappedSwapchain)->GetRotation(pRotation);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetSourceSize(UINT Width, UINT Height)
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->SetSourceSize(Width, Height);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetSourceSize(UINT* pWidth, UINT* pHeight)
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->GetSourceSize(pWidth, pHeight);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetMaximumFrameLatency(UINT MaxLatency)
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->SetMaximumFrameLatency(MaxLatency);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetMaximumFrameLatency(UINT* pMaxLatency)
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->GetMaximumFrameLatency(pMaxLatency);
}

HANDLE STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetFrameLatencyWaitableObject()
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->GetFrameLatencyWaitableObject();
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetMatrixTransform(const DXGI_MATRIX_3X2_F* pMatrix)
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->SetMatrixTransform(pMatrix);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetMatrixTransform(DXGI_MATRIX_3X2_F* pMatrix)
{
	return static_cast<IDXGISwapChain2*>(WrappedSwapchain)->GetMatrixTransform(pMatrix);
}

UINT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::GetCurrentBackBufferIndex()
{
	return static_cast<IDXGISwapChain3*>(WrappedSwapchain)->GetCurrentBackBufferIndex();
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE ColorSpace, UINT* pColorSpaceSupport)
{
	return static_cast<IDXGISwapChain3*>(WrappedSwapchain)->CheckColorSpaceSupport(ColorSpace, pColorSpaceSupport);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetColorSpace1(DXGI_COLOR_SPACE_TYPE ColorSpace)
{
	return static_cast<IDXGISwapChain3*>(WrappedSwapchain)->SetColorSpace1(ColorSpace);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT Format, UINT SwapChainFlags, const UINT* pCreationNodeMask, IUnknown* const* ppPresentQueue)
{
	return static_cast<IDXGISwapChain3*>(WrappedSwapchain)->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags, pCreationNodeMask, ppPresentQueue);
}

HRESULT STDMETHODCALLTYPE FStreamlineDXGISwapChainProxy::SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void* pMetaData)
{
	return static_cast<IDXGISwapChain4*>(WrappedSwapchain)->SetHDRMetaData(Type, Size, pMetaData);
}
