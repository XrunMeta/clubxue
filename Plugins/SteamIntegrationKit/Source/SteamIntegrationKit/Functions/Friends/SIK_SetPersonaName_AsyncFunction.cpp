

#include "SIK_SetPersonaName_AsyncFunction.h"

#include "Async/Async.h"

USIK_SetPersonaName_AsyncFunction* USIK_SetPersonaName_AsyncFunction::SetPersonaName(const FString& PersonaName)
{
	USIK_SetPersonaName_AsyncFunction* BlueprintNode = NewObject<USIK_SetPersonaName_AsyncFunction>();
	BlueprintNode->m_PersonaName = PersonaName;
	return BlueprintNode;
}

#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
void USIK_SetPersonaName_AsyncFunction::OnSetPersonaName(PersonaStateChange_t* PersonaStateChange, bool bIOFailure)
{
	auto Param = *PersonaStateChange;
	AsyncTask(ENamedThreads::GameThread, [this, Param, bIOFailure]()
	{
		if(bIOFailure)
		{
			OnFailure.Broadcast();
		}
		else
		{
			OnSuccess.Broadcast();
		}
	});
	SetReadyToDestroy();
	MarkAsGarbage();
}
#endif

void USIK_SetPersonaName_AsyncFunction::Activate()
{
	Super::Activate();
#if (WITH_ENGINE_STEAM && ONLINESUBSYSTEMSTEAM_PACKAGE) || (WITH_STEAMKIT && !WITH_ENGINE_STEAM)
	if(SteamFriends() == nullptr)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		MarkAsGarbage();
		return;
	}

#else
	OnFailure.Broadcast();
	SetReadyToDestroy();
	MarkAsGarbage();
#endif
}

