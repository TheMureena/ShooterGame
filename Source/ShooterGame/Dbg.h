#pragma once


#include "CoreMinimal.h"
#include "VisualLogger/VisualLogger.h"
#include "Dbg.generated.h"


static constexpr uint8 s_FLAG_DEBUG_LOG_TO_FILE = 0x01;
static constexpr uint8 s_FLAG_DEBUG_LOG_TO_SCREEN = 0x02;
static constexpr uint8 s_FLAG_DEBUG_LOG_TO_WORLD = 0x04;
static constexpr uint8 s_FLAG_DEBUG_LOG_TO_VLOGGER = 0x08;


static constexpr uint64 S_LOG_NORMAL = s_FLAG_DEBUG_LOG_TO_FILE | s_FLAG_DEBUG_LOG_TO_VLOGGER;
static constexpr uint64 s_LOG_IN_GAME = s_FLAG_DEBUG_LOG_TO_FILE | s_FLAG_DEBUG_LOG_TO_VLOGGER | s_FLAG_DEBUG_LOG_TO_WORLD;
static constexpr uint64 s_LOG_TO_SCREEN = s_FLAG_DEBUG_LOG_TO_FILE | s_FLAG_DEBUG_LOG_TO_VLOGGER | s_FLAG_DEBUG_LOG_TO_SCREEN;



UCLASS()
class UDbg : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_INFO_STR(const FString& Str);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_WARNING_STR(const FString& Str);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_ERROR_STR(const FString& Str);


	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_INFO_LOCATION(const FString& Str, const UObject* pOwner, const FVector& vPos, const float fRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_WARNING_LOCATION(const FString& Str, const UObject* pOwner, const FVector& vPos, const float fRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_ERROR_LOCATION(const FString& Str, const UObject* pOwner, const FVector& vPos, const float fRadius = 100.0f);


	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_INFO_ACTOR(const FString& Str, const AActor* pActor, const float fRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_WARNING_ACTOR(const FString& Str, const AActor* pActor, const float fRadius = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_ERROR_ACTOR(const FString& Str, const AActor* pActor, const float fRadius = 100.0f);


	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_INFO_LINE(const FString& Str, const UObject* pOwner, const FVector& vStart, const FVector& vEnd, const float fThickness = 5.0f, const bool bArrow = false);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_WARNING_LINE(const FString& Str, const UObject* pOwner, const FVector& vStart, const FVector& vEnd, const float fThickness = 5.0f, const bool bArrow = false);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_ERROR_LINE(const FString& Str, const UObject* pOwner, const FVector& vStart, const FVector& vEnd, const float fThickness = 5.0f, const bool bArrow = false);

	/** If an actor is passed, positions are relative to the actor's location. If Actor is null, positions should be in world space! **/
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static void LOG_STR_TO_SCREEN(const FString& sStr, AActor* pActor, const FVector& vPos, const FColor& vCol);


private:
	static void Log(const FString& Str, const FVector& vPos, const uint8 iLogFlag, int iMethod, const UObject* pOwner = nullptr);
};
