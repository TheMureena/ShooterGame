#include "Dbg.h"
#include "ShooterGame.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/HUD.h"



void UDbg::LOG_INFO_STR(const FString& Str)
{
	UE_LOG(ShooterGame_INFO, Log, TEXT("%s"), *Str);
}



void UDbg::LOG_WARNING_STR(const FString& Str)
{
	UE_LOG(ShooterGame_WARN, Warning, TEXT("%s"), *Str);
}



void UDbg::LOG_ERROR_STR(const FString& Str)
{
	UE_LOG(ShooterGame_CRIT, Error, TEXT("%s"), *Str);
}



void UDbg::LOG_INFO_LOCATION(const FString& Str, const UObject* pOwner, const FVector& vPos, const float fRadius)
{
	Log(Str, vPos, S_LOG_NORMAL, 0, pOwner);
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(pOwner, ShooterGame_INFO, Log, vPos, fRadius, FColor::White, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_WARNING_LOCATION(const FString& Str, const UObject* pOwner, const FVector& vPos, const float fRadius)
{
	Log(Str, vPos, S_LOG_NORMAL, 0, pOwner);
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(pOwner, ShooterGame_WARN, Log, vPos, fRadius, FColor::Orange, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_ERROR_LOCATION(const FString& Str, const UObject* pOwner, const FVector& vPos, const float fRadius)
{
	Log(Str, vPos, S_LOG_NORMAL, 0, pOwner);
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(pOwner, ShooterGame_CRIT, Log, vPos, fRadius, FColor::Red, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_INFO_LINE(const FString& Str, const UObject* pOwner, const FVector& vStart, const FVector& vEnd, const float fThickness, const bool bArrow)
{
	Log(Str, vStart, S_LOG_NORMAL, 0, pOwner);
#if ENABLE_VISUAL_LOG
	if (bArrow) UE_VLOG_ARROW(pOwner, ShooterGame_INFO, Log, vStart, vEnd, FColor::Black, TEXT("- %s"), *Str);
	else UE_VLOG_SEGMENT_THICK(pOwner, ShooterGame_INFO, Log, vStart, vEnd, FColor::Black, fThickness, TEXT("- %s"), *Str);

#endif
}



void UDbg::LOG_WARNING_LINE(const FString& Str, const UObject* pOwner, const FVector& vStart, const FVector& vEnd, const float fThickness, const bool bArrow)
{
	Log(Str, vStart, S_LOG_NORMAL, 1, pOwner);
#if ENABLE_VISUAL_LOG
	if (bArrow) UE_VLOG_ARROW(pOwner, ShooterGame_INFO, Log, vStart, vEnd, FColor::Orange, TEXT("- %s"), *Str);
	else UE_VLOG_SEGMENT_THICK(pOwner, ShooterGame_INFO, Log, vStart, vEnd, FColor::Orange, fThickness, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_ERROR_LINE(const FString& Str, const UObject* pOwner, const FVector& vStart, const FVector& vEnd, const float fThickness, const bool bArrow)
{
	Log(Str, vStart, S_LOG_NORMAL, 2, pOwner);
#if ENABLE_VISUAL_LOG
	if (bArrow) UE_VLOG_ARROW(pOwner, ShooterGame_INFO, Log, vStart, vEnd, FColor::Red, TEXT("- %s"), *Str);
	else UE_VLOG_SEGMENT_THICK(pOwner, ShooterGame_INFO, Log, vStart, vEnd, FColor::Red, fThickness, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_INFO_ACTOR(const FString& Str, const AActor* pActor, const float fRadius)
{
	Log(Str, FVector::ZeroVector, S_LOG_NORMAL, 0, Cast<UObject>(pActor));
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(pActor, ShooterGame_INFO, Log, pActor->GetActorLocation(), fRadius, FColor::White, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_WARNING_ACTOR(const FString& Str, const AActor* pActor, const float fRadius)
{
	Log(Str, FVector::ZeroVector, S_LOG_NORMAL, 1, Cast<UObject>(pActor));
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(pActor, ShooterGame_WARN, Log, pActor->GetActorLocation(), fRadius, FColor::Orange, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_ERROR_ACTOR(const FString& Str, const AActor* pActor, const float fRadius)
{
	Log(Str, FVector::ZeroVector, S_LOG_NORMAL, 2, Cast<UObject>(pActor));
#if ENABLE_VISUAL_LOG
	UE_VLOG_LOCATION(pActor, ShooterGame_CRIT, Log, pActor->GetActorLocation(), fRadius, FColor::Red, TEXT("- %s"), *Str);
#endif
}



void UDbg::LOG_STR_TO_SCREEN(const FString& sStr, AActor* pActor, const FVector& vPos, const FColor& vCol)
{
	APlayerController* PlayerController = pActor->GetWorld()->GetFirstPlayerController();
	if (sStr.Len() == 0)
	{
		if (PlayerController && PlayerController->MyHUD && PlayerController->Player) PlayerController->MyHUD->AddDebugText(" ", pActor, 1.0f / 30.0f, vPos, vPos, vCol, true, /*bAbsoluteLocation=*/ (pActor == nullptr), /*bKeepAttachedToActor=*/ true, nullptr, 1.75, true);
	}
	else
	{
		if (PlayerController && PlayerController->MyHUD && PlayerController->Player) PlayerController->MyHUD->AddDebugText(sStr, pActor, 1.0f / 30.0f, vPos, vPos, vCol, true, /*bAbsoluteLocation=*/ (pActor == nullptr), /*bKeepAttachedToActor=*/ true, nullptr, 1.75, true);
	}
}



void UDbg::Log(const FString& Str, const FVector& vPos, const uint8 iLogFlag, int iMethod, const UObject* pOwner)
{
	if (nullptr == GEngine) return;
	const auto* pWorld = GEngine->GetWorld();

	if (TEST_BIT(iLogFlag, s_FLAG_DEBUG_LOG_TO_FILE))
	{

		switch (iMethod)
		{
		case 0: UE_LOG(ShooterGame_INFO, Log, TEXT("%s"), *Str); break;
		case 1: UE_LOG(ShooterGame_WARN, Log, TEXT("%s"), *Str); break;
		case 2: UE_LOG(ShooterGame_CRIT, Log, TEXT("%s"), *Str); break;
		default: break;
		}
	}

	if (TEST_BIT(iLogFlag, s_FLAG_DEBUG_LOG_TO_SCREEN)) GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, Str);
	if (TEST_BIT(iLogFlag, s_FLAG_DEBUG_LOG_TO_WORLD)) DrawDebugString(pWorld, vPos, Str, nullptr, FColor::Green, 10.0f, false);
#if ENABLE_VISUAL_LOG
	FVisualLogger& vLog = FVisualLogger::Get();
	if (TEST_BIT(iLogFlag, s_FLAG_DEBUG_LOG_TO_VLOGGER) && vLog.IsRecording()) UE_VLOG(pOwner, ShooterGame_INFO, Log, TEXT("- %s"), *Str);
#endif
}

