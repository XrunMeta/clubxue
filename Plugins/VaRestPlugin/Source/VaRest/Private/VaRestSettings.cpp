

#include "VaRestSettings.h"

UVaRestSettings::UVaRestSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bExtendedLog = false;
	bUseChunkedParser = false;
}
