

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

class UBlueprintGeneratedClass;

namespace VisualStudioTools 
{
namespace AssetHelpers
{
void SetBlueprintClassFilter(FARFilter& InOutFilter);

void ForEachAsset(
	const TArray<FAssetData>& TargetAssets,
	TFunctionRef<void(UBlueprintGeneratedClass*, const FAssetData& AssetData)> Callback);

} 
} 
