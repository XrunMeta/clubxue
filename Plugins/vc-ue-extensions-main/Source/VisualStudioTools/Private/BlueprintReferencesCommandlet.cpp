

#include "BlueprintReferencesCommandlet.h"

#include "Algo/Find.h"
#include "Algo/Transform.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "BlueprintAssetHelpers.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "FindInBlueprintManager.h"
#include "JsonObjectConverter.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "VisualStudioTools.h"

namespace VisualStudioTools
{
static FString StripClassPrefix(const FString& InClassName)
{
	if (InClassName.IsEmpty())
	{
		return InClassName;
	}

	size_t PrefixSize = 0;

	const TCHAR ClassPrefixChar = InClassName[0];
	switch (ClassPrefixChar)
	{
	case TEXT('I'):
	case TEXT('A'):
	case TEXT('U'):

		if (InClassName.Len() > 12 && FCString::Strncmp(&(InClassName[1]), TEXT("DEPRECATED_"), 11) == 0)
		{
			PrefixSize = 12;
		}
		else
		{
			PrefixSize = 1;
		}
		break;
	case TEXT('F'):
	case TEXT('T'):

		PrefixSize = 1;
		break;
	default:
		PrefixSize = 0;
		break;
	}

	return InClassName.RightChop(PrefixSize);
}

TArray<FAssetData> SearchForCandidateAssets(const FString& SearchQuery)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	AssetRegistry.SearchAllAssets(true);

	TArray<FSearchResult> OutItemsFound;
	FStreamSearch StreamSearch(SearchQuery);
	while (!StreamSearch.IsComplete())
	{
		FFindInBlueprintSearchManager::Get().Tick(0.0);
	}

	StreamSearch.GetFilteredItems(OutItemsFound);

	TArray<FAssetData> OutTargetAssets;
	Algo::Transform(OutItemsFound, OutTargetAssets,
		[&](const FSearchResult& Item)
		{

#if FILTER_ASSETS_BY_CLASS_PATH
			return AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(*Item->DisplayText.ToString()));
#else
			return AssetRegistry.GetAssetByObjectPath(*Item->DisplayText.ToString());
#endif 
		});

	return OutTargetAssets;
}

TMap<FString, FAssetData> GetConfirmedAssets(
	const FString& FunctionName, const FString& ClassNameWithoutPrefix, const TArray<FAssetData>& InAssets)
{
	TMap<FString, FAssetData> OutResults;

	AssetHelpers::ForEachAsset(InAssets,
		[&](UBlueprintGeneratedClass* BlueprintClassName, const FAssetData AssetData)
		{
			auto MatchingFunction = Algo::FindByPredicate(BlueprintClassName->CalledFunctions,
				[&](const UFunction* Fn)
				{
					return Fn->HasAnyFunctionFlags(EFunctionFlags::FUNC_Native)
						&& Fn->GetName() == FunctionName
						&& Fn->GetOwnerClass()->GetName() == ClassNameWithoutPrefix;
				});

			if (MatchingFunction != nullptr)
			{
				OutResults.Add(BlueprintClassName->GetName(), AssetData);
			}
		});

	return OutResults;
}
using JsonWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;

static void SerializeBlueprintReference(
	TSharedRef<JsonWriter>& Json, const FString& BlueprintClassName, const FAssetData& Asset)
{
	FString PackageFileName;
	FString PackageFile;
	FString PackageFilePath;
	if (FPackageName::TryConvertLongPackageNameToFilename(Asset.GetPackage()->GetName(), PackageFileName) &&
		FPackageName::FindPackageFileWithoutExtension(PackageFileName, PackageFile))
	{
		PackageFilePath = FPaths::ConvertRelativePathToFull(MoveTemp(PackageFile));
	}

	Json->WriteObjectStart();
	Json->WriteValue(TEXT("name"), BlueprintClassName);
	Json->WriteValue(TEXT("path"), PackageFilePath);
	Json->WriteObjectEnd();
}

static void SerializeBlueprints(
	TSharedRef<JsonWriter>& Json, const TMap<FString, FAssetData>& InAssets)
{
	Json->WriteIdentifierPrefix(TEXT("blueprints"));
	Json->WriteArrayStart();

	for (auto& Item : InAssets)
	{
		const FString& BlueprintClassName = Item.Key;
		const FAssetData& Asset = Item.Value;
		SerializeBlueprintReference(Json, BlueprintClassName, Asset);
	}

	Json->WriteArrayEnd();
}

static void SerializeMetadata(
	TSharedRef<JsonWriter>& Json, int TotalAssetCount)
{
	Json->WriteIdentifierPrefix(TEXT("metadata"));
	Json->WriteObjectStart();
	{
		Json->WriteValue(TEXT("asset_count"), TotalAssetCount);
	}
	Json->WriteObjectEnd();
}

static void SerializeResults(
	const TMap<FString, FAssetData>& InAssets,
	FArchive& OutArchive,
	int TotalAssetCount)
{
	TSharedRef<JsonWriter> Json = JsonWriter::Create(&OutArchive);
	Json->WriteObjectStart();

	SerializeBlueprints(Json, InAssets);
	SerializeMetadata(Json, TotalAssetCount);

	Json->WriteObjectEnd();
	Json->Close();
}
} 

static constexpr auto SymbolParamVal = TEXT("symbol");

UVsBlueprintReferencesCommandlet::UVsBlueprintReferencesCommandlet()
	: Super()
{
	HelpDescription = TEXT("Commandlet for generating data used by Blueprint support in Visual Studio.");

	HelpParamNames.Add(SymbolParamVal);
	HelpParamDescriptions.Add(TEXT("[Optional] Fully qualified symbol to search for in the blueprints."));

	HelpUsage = TEXT("<Editor-Cmd.exe> <path_to_uproject> -run=VsBlueprintReferences -output=<path_to_output_file> -symbol=<ClassName::FunctionName> [-unattended -noshadercompile -nosound -nullrhi -nocpuprofilertrace -nocrashreports -nosplash]");
}

int32 UVsBlueprintReferencesCommandlet::Run(
	TArray<FString>& Tokens,
	TArray<FString>& Switches,
	TMap<FString, FString>& ParamVals,
	FArchive& OutArchive)
{
	using namespace VisualStudioTools;
	GIsRunning = true; 

	FString* ReferencesSymbol = ParamVals.Find(SymbolParamVal);
	if (ReferencesSymbol->IsEmpty())
	{
		UE_LOG(LogVisualStudioTools, Error, TEXT("Missing required symbol parameter."));
		PrintHelp();
		return -1;
	}

	FString FunctionName;
	FString ClassNameNative;
	if (!ReferencesSymbol->Split(TEXT("::"), &ClassNameNative, &FunctionName))
	{
		UE_LOG(LogVisualStudioTools, Error, TEXT("Reference parameter should be in the qualified 'NativeClassName::MethodName' format."));
		PrintHelp();
		return -1;
	}

	FString ClassNameWithoutPrefix = StripClassPrefix(ClassNameNative);

	FString SearchValue = FString::Printf(TEXT("Nodes(\"Native Name\"=+%s & ClassName=K2Node_CallFunction)"), *FunctionName);

	UE_LOG(LogVisualStudioTools, Display, TEXT("Blueprint search query: %s"), *SearchValue);

	TArray<FAssetData> TargetAssets = SearchForCandidateAssets(SearchValue);

	TMap<FString, FAssetData> MatchAssets = GetConfirmedAssets(FunctionName, ClassNameWithoutPrefix, TargetAssets);

	SerializeResults(MatchAssets, OutArchive, TargetAssets.Num());

	UE_LOG(LogVisualStudioTools, Display, TEXT("Found %d blueprints."), MatchAssets.Num());
	return 0;
}