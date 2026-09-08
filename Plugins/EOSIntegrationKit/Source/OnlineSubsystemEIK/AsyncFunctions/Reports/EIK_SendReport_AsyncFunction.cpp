

#include "EIK_SendReport_AsyncFunction.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemEOS.h"

UEIK_SendReport_AsyncFunction* UEIK_SendReport_AsyncFunction::SendEIKPlayerReportAsyncFunction(FString LocalReporterPUID, FString TargetPlayerPUID, E_PlayerReportCategory ReportCategory, FString Message)
{
	UEIK_SendReport_AsyncFunction* UEIK_SendReport_Object = NewObject < UEIK_SendReport_AsyncFunction>();
	UEIK_SendReport_Object->LocalReporterPUID = LocalReporterPUID;
	UEIK_SendReport_Object->TargetPlayerPUID = TargetPlayerPUID;
	UEIK_SendReport_Object->ReportCategory = ReportCategory;
	UEIK_SendReport_Object->Message = Message;
	return UEIK_SendReport_Object;
}

void UEIK_SendReport_AsyncFunction::SendReportFunc()
{
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		if (FOnlineSubsystemEOS* EOSRef = static_cast<FOnlineSubsystemEOS*>(OnlineSub))
		{
			if (EOSRef->ReportsHandle != nullptr)
			{
				EOS_Reports_SendPlayerBehaviorReportOptions ReportOptions;

				ReportOptions.ApiVersion = EOS_REPORTS_SENDPLAYERBEHAVIORREPORT_API_LATEST;
				ReportOptions.ReporterUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*LocalReporterPUID));
				ReportOptions.ReportedUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*TargetPlayerPUID));

				switch (ReportCategory)
				{

				case E_PlayerReportCategory::EOS_PRC_Cheating:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_Cheating;
					break;

				case E_PlayerReportCategory::EOS_PRC_Exploiting:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_Exploiting;
					break;

				case E_PlayerReportCategory::EOS_PRC_OffensiveProfile:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_OffensiveProfile;
					break;

				case E_PlayerReportCategory::EOS_PRC_VerbalAbuse:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_VerbalAbuse;
					break;

				case E_PlayerReportCategory::EOS_PRC_Scamming:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_Scamming;
					break;

				case E_PlayerReportCategory::EOS_PRC_Spamming:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_Spamming;
					break;

				case E_PlayerReportCategory::EOS_PRC_Other:
					ReportOptions.Category = EOS_EPlayerReportsCategory::EOS_PRC_Other;
					break;
				}

				ReportOptions.Context = "{}"; 
				FTCHARToUTF8 MessageUtf8(*Message);
				ReportOptions.Message = MessageUtf8.Get();

				EOS_Reports_SendPlayerBehaviorReport(EOSRef->ReportsHandle, &ReportOptions, this, SendReportFuncCallback);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("EOSRef->ReportsHandle = nullptr"));
			}
		}
		else
		{
			ResultFaliure();
		}
	}
	else
	{
		ResultFaliure();
	}
}

void UEIK_SendReport_AsyncFunction::SendReportFuncCallback(const EOS_Reports_SendPlayerBehaviorReportCompleteCallbackInfo* Data)
{
	if (Data->ClientData)
	{  

		if (UEIK_SendReport_AsyncFunction* SendReportFunction = static_cast<UEIK_SendReport_AsyncFunction*>(Data->ClientData))
		{

			if (Data->ResultCode == EOS_EResult::EOS_Success)
			{
				SendReportFunction->ResultSuccess();
			}
			else
			{
				SendReportFunction->ResultFaliure();
			}
		}
		else 
		{
			UE_LOG(LogTemp, Error, TEXT("UEIK_SendReport_AsyncFunction is null. No callback will be fired"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClientData is null. No callback will be fired"));
	}

}

void UEIK_SendReport_AsyncFunction::ResultFaliure()
{
	Failure.Broadcast();
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEIK_SendReport_AsyncFunction::ResultSuccess()
{
	Success.Broadcast();
	SetReadyToDestroy();
#if ENGINE_MAJOR_VERSION == 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void UEIK_SendReport_AsyncFunction::Activate()
{
	SendReportFunc();
	Super::Activate();
}
