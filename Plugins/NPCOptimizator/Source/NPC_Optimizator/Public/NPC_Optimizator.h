

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Modules/ModuleManager.h"
#include "Engine/World.h"

class FOptimizationManager; 

class FNPC_OptimizatorModule : public IModuleInterface, public FGCObject
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;

public:
	FOptimizationManager* GetOptimizationManagerForWorld(UWorld* World);

	FORCEINLINE static FOptimizationManager* GetOptimizationManager(UWorld* World)
	{
		FNPC_OptimizatorModule& NPC_OptimizatorModule = FModuleManager::LoadModuleChecked<FNPC_OptimizatorModule>("NPC_Optimizator");
		return NPC_OptimizatorModule.GetOptimizationManagerForWorld(World);
	}
private:
	void HandleWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	FDelegateHandle PreWorldInitializationHandle;
	FDelegateHandle PostWorldCleanupHandle;

	TMap<TObjectPtr<UWorld>, FOptimizationManager*> WorldOptimizers;
};
