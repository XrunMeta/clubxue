

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemEIK/SdkFunctions/EIK_SharedFunctionFile.h"
#include "Subsystems/GameInstanceSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include "eos_rtc.h"
THIRD_PARTY_INCLUDES_END
#include "EIK_RTCSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FEIK_RTC_ParticipantStatusChangedCallbackInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | RTC Interface")
	FEIK_ProductUserId LocalUserId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | RTC Interface")
	FString RoomName;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | RTC Interface")
	FEIK_ProductUserId ParticipantId;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | RTC Interface")
	TEnumAsByte<EEIK_ERTCParticipantStatus> ParticipantStatus;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | RTC Interface")
	TArray<FEIK_RTC_ParticipantMetadata> ParticipantMetadata;

	UPROPERTY(BlueprintReadWrite, Category = "EOS Integration Kit | SDK Functions | RTC Interface")
	bool bParticipantInBlocklist;

	FEIK_RTC_ParticipantStatusChangedCallbackInfo()
	{
		ParticipantStatus = EEIK_ERTCParticipantStatus::EIK_RTCPS_Left;
		bParticipantInBlocklist = false;
	}
	FEIK_RTC_ParticipantStatusChangedCallbackInfo(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data)
	{
		LocalUserId = Data->LocalUserId;
		RoomName = UTF8_TO_TCHAR(Data->RoomName);
		ParticipantId = Data->ParticipantId;
		ParticipantStatus = static_cast<EEIK_ERTCParticipantStatus>(Data->ParticipantStatus);
		bParticipantInBlocklist = Data->bParticipantInBlocklist ? true : false;
		for (uint32_t i = 0; i < Data->ParticipantMetadataCount; i++)
		{
			ParticipantMetadata.Add(Data->ParticipantMetadata[i]);
		}
	}

};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_RTC_OnDisconnectedCallback, const TEnumAsByte<EEIK_Result>, Result, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEIK_RTC_OnParticipantStatusChangedCallback, const FEIK_RTC_ParticipantStatusChangedCallbackInfo&, Data);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_RTC_OnRoomStatisticsUpdatedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const FString&, Statistic);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEIK_RTC_OnBlockParticipantCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEIK_ProductUserId&, ParticipantId, bool, bBlocked);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_RTC_OnJoinRoomCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const TArray<FEIK_RTC_Option>&, Options);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_RTC_OnLeaveRoomCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEIK_RTCAdmin_OnKickCompleteCallback, const TEnumAsByte<EEIK_Result>, ResultCode);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FString&, RoomName, const FEIK_ProductUserId&, LocalUserId, const TArray<FEIK_RTCAdmin_UserToken>&, UserTokens);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_RTCAudio_OnAudioBeforeRenderCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEIK_RTCAudio_AudioBuffer&, AudioBuffer, const FEIK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE(FEIK_RTCAudio_OnAudioDevicesChangedCallback);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_RTCAudio_OnAudioInputStateCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const TEnumAsByte<EEIK_ERTCAudioInputStatus>&, AudioInputState);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FEIK_RTCAudio_OnAudioOutputStateCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const TEnumAsByte<EEIK_ERTCAudioOutputStatus>&, AudioOutputState);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FEIK_RTCAudio_OnParticipantUpdatedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEIK_ProductUserId&, ParticipantId, bool,bSpeaking, const TEnumAsByte<EEIK_ERTCAudioStatus>&, AudioStatus);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEIK_RTCAudio_OnQueryInputDevicesInformationCallback, const TEnumAsByte<EEIK_Result>, ResultCode);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEIK_RTCAudio_OnQueryOutputDevicesInformationCallback, const TEnumAsByte<EEIK_Result>, ResultCode);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnRegisterPlatformUserCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FString&, PlatformUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnSetInputDeviceSettingsCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FString&, DeviceId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnSetOutputDeviceSettingsCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FString&, DeviceId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnUnregisterPlatformUserCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FString&, PlatformUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnUpdateParticipantVolumeCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnUpdateReceivingCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnUpdateReceivingVolumeCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnUpdateSendingCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCAudio_OnUpdateSendingVolumeCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_RTCData_OnDataReceivedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const TArray<uint8>&, Data, const FEIK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_RTCData_OnParticipantUpdatedCallback, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEIK_ProductUserId&, ParticipantId, const TEnumAsByte<EEIK_ERTCDataStatus>&, DataStatus);
DECLARE_DYNAMIC_DELEGATE_FourParams(FEIK_RTCData_OnUpdateReceivingCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId, const FString&, RoomName, const FEIK_ProductUserId&, ParticipantId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEIK_RTCData_OnUpdateSendingCallback, const TEnumAsByte<EEIK_Result>, ResultCode, const FEIK_ProductUserId&, LocalUserId);
UCLASS(DisplayName = "RTC Interface", meta = (DisplayName = "RTC Interface"))
class ONLINESUBSYSTEMEIK_API UEIK_RTCSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	FEIK_RTC_OnDisconnectedCallback OnDisconnectedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_AddNotifyDisconnected")
	FEIK_NotificationId EIK_RTC_AddNotifyDisconnected(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTC_OnDisconnectedCallback& Callback);

	FEIK_RTC_OnParticipantStatusChangedCallback OnParticipantStatusChangedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_AddNotifyParticipantStatusChanged")
	FEIK_NotificationId EIK_RTC_AddNotifyParticipantStatusChanged(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTC_OnParticipantStatusChangedCallback& Callback);

	FEIK_RTC_OnRoomStatisticsUpdatedCallback OnRoomStatisticsUpdatedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_AddNotifyRoomStatisticsUpdated")
	FEIK_NotificationId EIK_RTC_AddNotifyRoomStatisticsUpdated(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTC_OnRoomStatisticsUpdatedCallback& Callback);

	FEIK_RTC_OnBlockParticipantCallback OnBlockParticipantCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_BlockParticipant")
	void EIK_RTC_BlockParticipant(FEIK_ProductUserId LocalUserId, const FString& RoomName, FEIK_ProductUserId ParticipantId, bool bBlocked, const FEIK_RTC_OnBlockParticipantCallback& Callback);

	FEIK_RTC_OnJoinRoomCallback OnJoinRoomCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_JoinRoom")
	void EIK_RTC_JoinRoom(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FString& ClientBaseUrl, const FString& ParticipantToken, FEIK_ProductUserId ParticipantId, bool bEnabledEcho, bool bManualAudioInputEnabled, bool bManualAudioOutputEnabled, const FEIK_RTC_OnJoinRoomCallback& Callback);

	FEIK_RTC_OnLeaveRoomCallback OnLeaveRoomCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_LeaveRoom")
	void EIK_RTC_LeaveRoom(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTC_OnLeaveRoomCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_RemoveNotifyDisconnected")
	void EIK_RTC_RemoveNotifyDisconnected(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_RemoveNotifyParticipantStatusChanged")
	void EIK_RTC_RemoveNotifyParticipantStatusChanged(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_RemoveNotifyRoomStatisticsUpdated")
	void EIK_RTC_RemoveNotifyRoomStatisticsUpdated(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_SetRoomSetting")
	TEnumAsByte<EEIK_Result> EIK_RTC_SetRoomSetting(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FString& SettingName, const FString& SettingValue);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTC_SetSetting")
	TEnumAsByte<EEIK_Result> EIK_RTC_SetSetting(const FString& SettingName, const FString& SettingValue);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_CopyUserTokenByIndex")
	TEnumAsByte<EEIK_Result> EIK_RTCAdmin_CopyUserTokenByIndex(int32 UserTokenIndex, int32 QueryId, FEIK_RTCAdmin_UserToken& OutUserToken);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_CopyUserTokenByUserId")
	TEnumAsByte<EEIK_Result> EIK_RTCAdmin_CopyUserTokenByUserId(FEIK_ProductUserId UserId, int32 QueryId, FEIK_RTCAdmin_UserToken& OutUserToken);

	FEIK_RTCAdmin_OnKickCompleteCallback OnKickCompleteCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_Kick")
	void EIK_RTCAdmin_Kick(const FString& RoomName, FEIK_ProductUserId TargetUserId, const FEIK_RTCAdmin_OnKickCompleteCallback& Callback);

	FEIK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback OnQueryJoinRoomTokenCompleteCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAdmin_QueryJoinRoomToken")
	TEnumAsByte<EEIK_Result> EIK_RTCAdmin_QueryJoinRoomToken(const FString& RoomName, FEIK_ProductUserId LocalUserId, const TArray<FEIK_ProductUserId>& TargetUserIds, TArray<FString>& TargetUserIpAddresses, const FEIK_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback& Callback);

	FEIK_RTCAudio_OnAudioBeforeRenderCallback OnAudioBeforeRenderCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioBeforeRender")
	FEIK_NotificationId EIK_RTCAudio_AddNotifyAudioBeforeRender(FEIK_ProductUserId LocalUserId, const FString& RoomName, bool bUnmixedAudio, const FEIK_RTCAudio_OnAudioBeforeRenderCallback& Callback);

	FEIK_RTCAudio_OnAudioDevicesChangedCallback OnAudioDevicesChangedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioDevicesChanged")
	FEIK_NotificationId EIK_RTCAudio_AddNotifyAudioDevicesChanged(const FEIK_RTCAudio_OnAudioDevicesChangedCallback& Callback);

	FEIK_RTCAudio_OnAudioInputStateCallback OnAudioInputStateCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioInputState")
	FEIK_NotificationId EIK_RTCAudio_AddNotifyAudioInputState(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTCAudio_OnAudioInputStateCallback& Callback);

	FEIK_RTCAudio_OnAudioOutputStateCallback OnAudioOutputStateCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyAudioOutputState")
	FEIK_NotificationId EIK_RTCAudio_AddNotifyAudioOutputState(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTCAudio_OnAudioOutputStateCallback& Callback);

	FEIK_RTCAudio_OnParticipantUpdatedCallback OnParticipantUpdatedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_AddNotifyParticipantUpdated")
	FEIK_NotificationId EIK_RTCAudio_AddNotifyParticipantUpdated(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTCAudio_OnParticipantUpdatedCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_CopyInputDeviceInformationByIndex")
	TEnumAsByte<EEIK_Result> EIK_RTCAudio_CopyInputDeviceInformationByIndex(int32 DeviceIndex, FEIK_RTCAudio_InputDeviceInformation& OutDeviceInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_CopyOutputDeviceInformationByIndex")
	TEnumAsByte<EEIK_Result> EIK_RTCAudio_CopyOutputDeviceInformationByIndex(int32 DeviceIndex, FEIK_RTCAudio_OutputDeviceInformation& OutDeviceInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_GetInputDevicesCount")
	int32 EIK_RTCAudio_GetInputDevicesCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_GetOutputDevicesCount")
	int32 EIK_RTCAudio_GetOutputDevicesCount();

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_InputDeviceInformation_Release")
	void EIK_RTCAudio_InputDeviceInformation_Release(FEIK_RTCAudio_InputDeviceInformation& DeviceInfo);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_OutputDeviceInformation_Release")
	void EIK_RTCAudio_OutputDeviceInformation_Release(FEIK_RTCAudio_OutputDeviceInformation& DeviceInfo);

	FEIK_RTCAudio_OnQueryInputDevicesInformationCallback OnQueryInputDevicesInformationCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_QueryInputDevicesInformation")
	void EIK_RTCAudio_QueryInputDevicesInformation(const FEIK_RTCAudio_OnQueryInputDevicesInformationCallback& Callback);

	FEIK_RTCAudio_OnQueryOutputDevicesInformationCallback OnQueryOutputDevicesInformationCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_QueryOutputDevicesInformation")
	void EIK_RTCAudio_QueryOutputDevicesInformation(const FEIK_RTCAudio_OnQueryOutputDevicesInformationCallback& Callback);

	FEIK_RTCAudio_OnRegisterPlatformUserCallback OnRegisterPlatformUserCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RegisterPlatformUser")
	void EIK_RTCAudio_RegisterPlatformUser(const FString& PlatformUserId, const FEIK_RTCAudio_OnRegisterPlatformUserCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioBeforeRender")
	void EIK_RTCAudio_RemoveNotifyAudioBeforeRender(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioBeforeSend")
	void EIK_RTCAudio_RemoveNotifyAudioBeforeSend(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioDevicesChanged")
	void EIK_RTCAudio_RemoveNotifyAudioDevicesChanged(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioInputState")
	void EIK_RTCAudio_RemoveNotifyAudioInputState(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyAudioOutputState")
	void EIK_RTCAudio_RemoveNotifyAudioOutputState(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_RemoveNotifyParticipantUpdated")
	void EIK_RTCAudio_RemoveNotifyParticipantUpdated(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_SendAudio")
	TEnumAsByte<EEIK_Result> EIK_RTCAudio_SendAudio(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTCAudio_AudioBuffer& AudioBuffer);

	FEIK_RTCAudio_OnSetInputDeviceSettingsCallback OnSetInputDeviceSettingsCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_SetInputDeviceSettings")
	void EIK_RTCAudio_SetInputDeviceSettings(FEIK_ProductUserId LocalUserId, const FString& RealDeviceId, bool bPlatformAEC, const FEIK_RTCAudio_OnSetInputDeviceSettingsCallback& Callback);

	FEIK_RTCAudio_OnSetOutputDeviceSettingsCallback OnSetOutputDeviceSettingsCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_SetOutputDeviceSettings")
	void EIK_RTCAudio_SetOutputDeviceSettings(FEIK_ProductUserId LocalUserId, const FString& RealDeviceId, const FEIK_RTCAudio_OnSetOutputDeviceSettingsCallback& Callback);

	FEIK_RTCAudio_OnUnregisterPlatformUserCallback OnUnregisterPlatformUserCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UnregisterPlatformUser")
	void EIK_RTCAudio_UnregisterPlatformUser(const FString& PlatformUserId, const FEIK_RTCAudio_OnUnregisterPlatformUserCallback& Callback);

	FEIK_RTCAudio_OnUpdateParticipantVolumeCallback OnUpdateParticipantVolumeCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateParticipantVolume")
	void EIK_RTCAudio_UpdateParticipantVolume(FEIK_ProductUserId LocalUserId, const FString& RoomName, FEIK_ProductUserId ParticipantId, float Volume, const FEIK_RTCAudio_OnUpdateParticipantVolumeCallback& Callback);

	FEIK_RTCAudio_OnUpdateReceivingCallback OnUpdateReceivingCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateReceiving")
	void EIK_RTCAudio_UpdateReceiving(FEIK_ProductUserId LocalUserId, const FString& RoomName, FEIK_ProductUserId ParticipantId, bool bAudioEnabled, const FEIK_RTCAudio_OnUpdateReceivingCallback& Callback);

	FEIK_RTCAudio_OnUpdateReceivingVolumeCallback OnUpdateReceivingVolumeCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateReceivingVolume")
	void EIK_RTCAudio_UpdateReceivingVolume(FEIK_ProductUserId LocalUserId, const FString& RoomName, float Volume, const FEIK_RTCAudio_OnUpdateReceivingVolumeCallback& Callback);

	FEIK_RTCAudio_OnUpdateSendingCallback OnUpdateSendingCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateSending")
	void EIK_RTCAudio_UpdateSending(FEIK_ProductUserId LocalUserId, const FString& RoomName, TEnumAsByte<EEIK_ERTCAudioStatus> AudioStatus, const FEIK_RTCAudio_OnUpdateSendingCallback& Callback);

	FEIK_RTCAudio_OnUpdateSendingVolumeCallback OnUpdateSendingVolumeCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCAudio_UpdateSendingVolume")
	void EIK_RTCAudio_UpdateSendingVolume(FEIK_ProductUserId LocalUserId, const FString& RoomName, float Volume, const FEIK_RTCAudio_OnUpdateSendingVolumeCallback& Callback);

	FEIK_RTCData_OnDataReceivedCallback OnDataReceivedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_AddNotifyDataReceived")
	FEIK_NotificationId EIK_RTCData_AddNotifyDataReceived(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTCData_OnDataReceivedCallback& Callback);

	FEIK_RTCData_OnParticipantUpdatedCallback OnData_ParticipantUpdatedCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_AddNotifyParticipantUpdated")
	FEIK_NotificationId EIK_RTCData_AddNotifyParticipantUpdated(FEIK_ProductUserId LocalUserId, const FString& RoomName, const FEIK_RTCData_OnParticipantUpdatedCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_RemoveNotifyDataReceived")
	void EIK_RTCData_RemoveNotifyDataReceived(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_RemoveNotifyParticipantUpdated")
	void EIK_RTCData_RemoveNotifyParticipantUpdated(FEIK_NotificationId NotificationId);

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_SendData")
	TEnumAsByte<EEIK_Result> EIK_RTCData_SendData(FEIK_ProductUserId LocalUserId, const FString& RoomName, const TArray<uint8>& Data);

	FEIK_RTCData_OnUpdateReceivingCallback OnData_UpdateReceivingCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_UpdateReceiving")
	void EIK_RTCData_UpdateReceiving(FEIK_ProductUserId LocalUserId, const FString& RoomName, FEIK_ProductUserId ParticipantId, bool bDataEnabled, const FEIK_RTCData_OnUpdateReceivingCallback& Callback);

	FEIK_RTCData_OnUpdateSendingCallback OnData_UpdateSendingCallback;

	UFUNCTION(BlueprintCallable, Category = "EOS Integration Kit | SDK Functions | RTC Interface", DisplayName="EOS_RTCData_UpdateSending")
	void EIK_RTCData_UpdateSending(FEIK_ProductUserId LocalUserId, const FString& RoomName, bool bDataEnabled, const FEIK_RTCData_OnUpdateSendingCallback& Callback);
};
