#include "ClubXEmailBPLibrary.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

void UClubXEmailBPLibrary::OpenEmail(const FString& Email)
{
    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] OpenEmail called: %s"), *Email);

#if PLATFORM_ANDROID

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] Platform Android"));

    JNIEnv* Env = FAndroidApplication::GetJavaEnv();

    if (!Env)
    {
        UE_LOG(LogTemp, Error, TEXT("[ClubXEmail] GetJavaEnv FAILED"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] JNI Env OK"));

    jclass GameActivityClass =
        FAndroidApplication::FindJavaClass("com/epicgames/unreal/GameActivity");

    if (!GameActivityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[ClubXEmail] GameActivity class NOT FOUND"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] GameActivity class OK"));

    jmethodID MethodID =
        Env->GetMethodID(
            GameActivityClass,
            "AndroidThunkJava_OpenClubXEmail",
            "(Ljava/lang/String;)V"
        );

    if (!MethodID)
    {
        UE_LOG(LogTemp, Error, TEXT("[ClubXEmail] AndroidThunkJava_OpenClubXEmail NOT FOUND"));
        Env->DeleteLocalRef(GameActivityClass);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] Java method OK"));

    jobject Activity =
        FAndroidApplication::GetGameActivityThis();

    if (!Activity)
    {
        UE_LOG(LogTemp, Error, TEXT("[ClubXEmail] GameActivity instance NULL"));
        Env->DeleteLocalRef(GameActivityClass);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] GameActivity instance OK"));

    jstring JEmail =
        Env->NewStringUTF(TCHAR_TO_UTF8(*Email));

    if (!JEmail)
    {
        UE_LOG(LogTemp, Error, TEXT("[ClubXEmail] Failed to create Java email string"));
        Env->DeleteLocalRef(GameActivityClass);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] Calling Java method"));

    Env->CallVoidMethod(
        Activity,
        MethodID,
        JEmail
    );

    if (Env->ExceptionCheck())
    {
        UE_LOG(LogTemp, Error, TEXT("[ClubXEmail] Java exception occurred"));
        Env->ExceptionDescribe();
        Env->ExceptionClear();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] Java method executed successfully"));
    }

    Env->DeleteLocalRef(JEmail);
    Env->DeleteLocalRef(GameActivityClass);

#else

    UE_LOG(LogTemp, Warning, TEXT("[ClubXEmail] Not Android"));

#endif
}