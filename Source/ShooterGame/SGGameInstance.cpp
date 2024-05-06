////

#include "SGGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

// Sets default values
USGGameInstance::USGGameInstance()
{
	MySessionName = FName("ML My Session");
}

void USGGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get())
	{
		SessionInterface = SubSystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			// Bind Delegates Here
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USGGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USGGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USGGameInstance::OnJoinSessionComplete);
		}
	}
}

void USGGameInstance::CreateServer(FCreateServerInfoSteam ServerInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateServer"));
	
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	//SessionSettings.bUseLobbiesIfAvailable = true;

	SessionSettings.bIsLANMatch = ServerInfo.bIsLAN;

	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
	{
		SessionSettings.bIsLANMatch = false; // SET false for Steam
	}
	else
	{
		SessionSettings.bIsLANMatch = true; // SET true for testing with def subsystem
	}

	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;

	if (ServerInfo.bHasPassword)
	{
		SessionSettings.NumPublicConnections = 0;
		SessionSettings.NumPrivateConnections = ServerInfo.MaxPlayers;
	}

	SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings.NumPublicConnections = ServerInfo.MaxPlayers;
	SessionSettings.bAllowInvites = true;
	SessionSettings.Set(FName("SERVER_NAME_KEY"), ServerInfo.ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(FName("SERVER_IS_PASSWORD_PROTECTED_KEY"), ServerInfo.ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(FName("SERVER_PASSWORD_KEY"), ServerInfo.Password, EOnlineDataAdvertisementType::DontAdvertise);

	SessionInterface->CreateSession(0, MySessionName, SessionSettings);
}

void USGGameInstance::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete, Succeeded: %d"), Succeeded);
	if (Succeeded)
	{
		GetWorld()->ServerTravel("/Game/Levels/Map1?listen");
	}
}

void USGGameInstance::OnFindSessionsComplete(bool Succeeded)
{
	SearchingDelegate.Broadcast(false);

	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete, Succeeded: %d"), Succeeded);
	if (Succeeded)
	{
		int32 ArrayIndex = 0;

		for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
		{
			if (!Result.IsValid())
				continue;

			FServerInfoSteam ServerInfo;
			FString ServerName = "";

			Result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName);

			ServerInfo.ServerName = ServerName;
			ServerInfo.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			ServerInfo.CurrentPlayers = ServerInfo.MaxPlayers - Result.Session.NumOpenPublicConnections;
			ServerInfo.bIsLAN = Result.Session.SessionSettings.bIsLANMatch;
			ServerInfo.Ping = Result.PingInMs;

			ServerInfo.ServersArrayIndex = ArrayIndex;

			ServerListDelegate.Broadcast(ServerInfo);
			ArrayIndex++;
		}

		UE_LOG(LogTemp, Warning, TEXT("SearchResults, Server Count: %d"), SessionSearch->SearchResults.Num());
	}
}

void USGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete, Succeeded: %d"), *SessionName.ToString());
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
		if (JoinAddress != "")
		{
			PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}

void USGGameInstance::OnDestroySessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete, Succeeded: %d"), Succeeded);
}

void USGGameInstance::FindServers()
{
	SearchingDelegate.Broadcast(true);

	UE_LOG(LogTemp, Warning, TEXT("Find Game Servers"));

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
	{
		SessionSearch->bIsLanQuery = false; // SET false for Steam
	}
	else
	{
		SessionSearch->bIsLanQuery = true; // SET true for testing with def subsystem
	}

	SessionSearch->MaxSearchResults = 10000; // huge bc steam search results for this appID is huge not visible though
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void USGGameInstance::JoinServer(int32 ServersArrayIndex)
{
	FOnlineSessionSearchResult Result = SessionSearch->SearchResults[ServersArrayIndex];
	if (Result.IsSessionInfoValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("JOINING SERVER AT INDEX: %d"), ServersArrayIndex);
		SessionInterface->JoinSession(0, MySessionName, Result);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FAILED TO JOIN SERVER AT INDEX: %d"), ServersArrayIndex);
	}
}

void USGGameInstance::UpdateLobbyInfo(FLobbyInfo NewLobbyInfo)
{
	LobbyInfo.MapName = NewLobbyInfo.MapName;
	LobbyInfo.MapImage = NewLobbyInfo.MapImage;
}

void USGGameInstance::GoToMap(FString MapURL)
{
	GetWorld()->ServerTravel(MapURL);
}