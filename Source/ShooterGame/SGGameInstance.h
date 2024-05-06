#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SGGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FCreateServerInfoSteam
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString ServerName = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapName = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsLAN = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bHasPassword = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Password = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxPlayers = 0;
};

USTRUCT(BlueprintType)
struct FServerInfoSteam
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString ServerName = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapName = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsLAN = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bHasPassword = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Ping = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ServersArrayIndex = 0;
};

USTRUCT(BlueprintType)
struct FMapInfo_Versus
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapURL = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapName = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapDescription = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UTexture2D* MapImage = nullptr;
};

USTRUCT(BlueprintType)
struct FMapInfo_SingleCoop
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapURL = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapName = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MapDescription = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UTexture2D* MapImage = nullptr;
};

USTRUCT(BlueprintType)
struct FLobbyInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FString MapName = "";

	UPROPERTY(BlueprintReadWrite)
	class UTexture2D* MapImage = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamServersReceived, FServerInfoSteam, ServerListDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSteamServersSearching, bool, SearchingDelegate);

UCLASS()
class SHOOTERGAME_API USGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this empty's properties
	USGGameInstance();

protected:

	FName MySessionName;

	FLobbyInfo LobbyInfo;

	UPROPERTY(BlueprintAssignable)
	FSteamServersReceived ServerListDelegate;

	UPROPERTY(BlueprintAssignable)
	FSteamServersSearching SearchingDelegate;

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);
	virtual void OnFindSessionsComplete(bool Succeeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool Succeeded);

	UFUNCTION(BlueprintCallable)
	void CreateServer(FCreateServerInfoSteam ServerInfo);

	UFUNCTION(BlueprintCallable)
	void FindServers();

	UFUNCTION(BlueprintCallable)
	void JoinServer(int32 ServersArrayIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateLobbyInfo(FLobbyInfo NewLobbyInfo);

protected:

	UFUNCTION(BlueprintCallable)
	void GoToMap(FString MapURL);

};
