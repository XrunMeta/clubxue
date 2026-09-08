

#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#include "RHIResources.h"

#include "StreamlineCorePrivate.h"

class FStreamlineDLSSGCustomPresent final : public FRHICustomPresent
{
public:

	virtual void OnBackBufferResize() override final;

	virtual bool NeedsNativePresent() override final { return true; }

	virtual bool NeedsAdvanceBackbuffer() override final { return true; }

#if !UE_VERSION_OLDER_THAN(5,8,0)
	virtual bool Present(FRHIViewport*, IRHICommandContext&, int32&) override final { return true; };
	virtual bool PresentOnSubmissionThread(int32& InOutSyncInterval) override final { return true; };
#elif !UE_VERSION_OLDER_THAN(5,5,0)
	virtual bool Present(IRHICommandContext& RHICmdContext, int32& InOutSyncInterval) override final { return true; };
#else
	virtual bool Present(int32& InOutSyncInterval) override final { return true; }
#endif

};

