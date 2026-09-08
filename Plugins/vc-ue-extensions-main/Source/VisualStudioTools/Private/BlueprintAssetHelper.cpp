

#include "BlueprintAssetHelpers.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/BlueprintSupport.h"
#include "Engine/BlueprintCore.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Misc/ScopeExit.h"
#include "VisualStudioTools.h"

namespace VisualStudioTools
{
namespace AssetHelpers
{

#if FILTER_ASSETS_BY_CLASS_PATH

void SetBlueprintClassFilter(FARFilter& InOutFilter)
{

	InOutFilter.ClassPaths.Add(UBlueprintCore::StaticClass()->GetClassPathName());
}

static FString GetObjectPathString(const FAssetData& InAssetData)
{

	return InAssetData.GetObjectPathString();
}

#else 

void SetBlueprintClassFilter(FARFilter& InOutFilter)
{
	InOutFilter.ClassNames.Add(UBlueprintCore::StaticClass()->GetFName());
}

static FString GetObjectPathString(const FAssetData& InAssetData)
{
	return InAssetData.ObjectPath.ToString();
}

#endif 

void ForEachAsset(
	const TArray<FAssetData>& TargetAssets,
	TFunctionRef<void(UBlueprintGeneratedClass*, const FAssetData& AssetData)> Callback)
{

	TGuardValue<bool> DisableLogVerbosity(GPrintLogVerbosity, false);
	TGuardValue<bool> DisableLogCategory(GPrintLogCategory, false);

	GEngine->Exec(nullptr, TEXT("log LogVisualStudioTools only"));
	ON_SCOPE_EXIT
	{
		GEngine->Exec(nullptr, TEXT("log reset"));
	};

	FStreamableManager AssetLoader;

	for (int32 Idx = 0; Idx < TargetAssets.Num(); Idx++)
	{
		const FAssetData AssetData = TargetAssets[Idx];
		FSoftClassPath GenClassPath = AssetData.GetTagValueRef<FString>(FBlueprintTags::GeneratedClassPath);
		UE_LOG(LogVisualStudioTools, Display, TEXT("Processing blueprints [%d/%d]: %s"), Idx + 1, TargetAssets.Num(), *GenClassPath.ToString());

		TSharedPtr<FStreamableHandle> Handle = AssetLoader.RequestSyncLoad(GenClassPath);
		ON_SCOPE_EXIT
		{

			Handle->ReleaseHandle();
		};

		if (!Handle.IsValid())
		{
			UE_LOG(LogVisualStudioTools, Warning, TEXT("Failed to get a streamable handle for Blueprint. Skipping. GenClassPath: %s"), *GenClassPath.ToString());
			continue;
		}

		if (auto BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(Handle->GetLoadedAsset()))
		{
			Callback(BlueprintGeneratedClass, AssetData);
		}
		else
		{

			FString ObjectPathString = AssetHelpers::GetObjectPathString(AssetData);

			FString Msg = !GenClassPath.ToString().Contains(ObjectPathString)
				? FString::Printf(
					TEXT("ObjectPath is not compatible with GenClassPath, consider re-saving it to avoid future issues. { ObjectPath: %s, GenClassPath: %s }"),
					*ObjectPathString,
					*GenClassPath.ToString())
				: FString::Printf(TEXT("ClassPath: %s"), *GenClassPath.ToString());

			UE_LOG(LogVisualStudioTools, Warning, TEXT("Failed to load Blueprint. Skipping. %s"), *Msg);
		}
	}
}

}
}