

#pragma once

#include "CoreMinimal.h"
#include "eos_reports.h"
#include "eos_reports_types.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EIK_SendReport_AsyncFunction.generated.h"

UENUM(BlueprintType)
enum class E_PlayerReportCategory : uint8
{

	EOS_PRC_Cheating  UMETA(DisplayName = "Cheating"),

	EOS_PRC_Exploiting  UMETA(DisplayName = "Exploiting"),

	EOS_PRC_OffensiveProfile  UMETA(DisplayName = "OffensiveProfile"),

	EOS_PRC_VerbalAbuse  UMETA(DisplayName = "VerbalAbuse"),

	EOS_PRC_Scamming  UMETA(DisplayName = "Scamming"),

	EOS_PRC_Spamming  UMETA(DisplayName = "Spamming"),

	EOS_PRC_Other  UMETA(DisplayName = "Other"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReportsDelegate);

UCLASS()
class ONLINESUBSYSTEMEIK_API UEIK_SendReport_AsyncFunction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	FString LocalReporterPUID;
	FString TargetPlayerPUID;

	E_PlayerReportCategory ReportCategory;

	FString Message;

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit || Reports")
	FReportsDelegate Success;

	UPROPERTY(BlueprintAssignable, Category = "EOS Integration Kit || Reports")
	FReportsDelegate Failure;

	UFUNCTION(BlueprintCallable, DisplayName = "Send EIK Player Report", meta = (BlueprintInternalUseOnly = "true"), Category = "EOS Integration Kit || Reports")
	static UEIK_SendReport_AsyncFunction* SendEIKPlayerReportAsyncFunction(FString LocalReporterPUID, FString TargetPlayerPUID, E_PlayerReportCategory ReportCategory, FString Message);

	void SendReportFunc();

	static void EOS_CALL SendReportFuncCallback(const EOS_Reports_SendPlayerBehaviorReportCompleteCallbackInfo* Data);

	void ResultFaliure();

	void ResultSuccess();

	void Activate() override;

};
