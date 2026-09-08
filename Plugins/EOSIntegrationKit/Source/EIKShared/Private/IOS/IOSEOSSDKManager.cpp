

#if WITH_EOS_SDK

#include "IOSEOSSDKManager.h"

FString FIOSEOSSDKManager::GetCacheDirBase() const
{
	NSString* BundleIdentifier = [[NSBundle mainBundle]bundleIdentifier];
	NSString* CacheDirectory = NSTemporaryDirectory(); 
	CacheDirectory = [CacheDirectory stringByAppendingPathComponent : BundleIdentifier];

	const char* CStrCacheDirectory = [CacheDirectory UTF8String];
	return FString(UTF8_TO_TCHAR(CStrCacheDirectory));
};

#endif 