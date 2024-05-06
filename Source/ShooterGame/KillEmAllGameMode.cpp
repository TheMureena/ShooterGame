// Fill out your copyright notice in the Description page of Project Settings.


#include "KillEmAllGameMode.h"
#include "EngineUtils.h"
#include "ShooterGameAIController.h"
#include "ShooterGamePlayerController.h"
#include "SGPlayerState.h"
#include "ShooterCharacter.h"
#include "MonsterMelee.h"
#include "EnemyRanged.h"
#include "SGEnemySpawnPoint.h"
#include "SGPlayerState.h"
//#include "SGEnemyTrigger.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

AKillEmAllGameMode::AKillEmAllGameMode()
{
	///AGameModeBase::bUseSeamlessTravel = true;
}

void AKillEmAllGameMode::BeginPlay()
{
	TArray<AActor*> TempActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASGEnemySpawnPoint::StaticClass(), TempActors);

	for (AActor* Actor : TempActors)
	{
		if (ASGEnemySpawnPoint* EnemySpawnPoint = Cast<ASGEnemySpawnPoint>(Actor))
		{
			ActiveEnemySpawnPoints.Add(EnemySpawnPoint);
			/*	if (ASGEnemyTrigger* LinkedTrigger = EnemySpawnPoint->GetLinkedTrigger())
				{
					SpawnPoint->SetZone(LinkedTrigger->GetActivateZone());
					AllEnemySpawnPoints.Add(EnemySpawnPoint);
					UE_LOG(LogTemp, Warning, TEXT("Zone Number: %d"), LinkedTrigger->GetActivateZone());
				}
				else
				{
				ActiveEnemySpawnPoints.Add(EnemySpawnPoint);

				}
			*/
		}
	}

	SpawnEnemy();

	TArray<AActor*> TempPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterCharacter::StaticClass(), TempPlayers);

	for (AActor* Actor : TempPlayers)
	{
		if (AShooterCharacter* Player = Cast<AShooterCharacter>(Actor))
		{
			ConnectedPlayers.Add(Player);
		}
	}
}

void AKillEmAllGameMode::PawnKilled(APawn* PawnKilled)
{
	Super::PawnKilled(PawnKilled);

	/// original single player version
	//APlayerController* PlayerController = Cast<APlayerController>(PawnKilled->GetController());
	//if (PlayerController != nullptr)
	//{
	//	EndGame(false);
	//}

	/// for multiple players this might not work bc we destroy the player controllers on death
	/// dont need to look for our own specific player controller because it is a player controller
	
	//APlayerController* DeadPawnsPlayerController = Cast<APlayerController>(PawnKilled->GetController());

	for (APlayerState* PState : GetGameState<AGameStateBase>()->PlayerArray)
	{
		if (ASGPlayerState* ShooterPState = Cast<ASGPlayerState>(PState))
		{
			if (!ShooterPState->IsDead())
			{
				return;
			}
			EndGame(false);
		}
	}

	//for (ASGPlayerState* PlayerState : TActorRange<ASGPlayerState>(GetWorld()))
	//{
	//	if (!PlayerState->IsDead())
	//	{
	//		return;
	//	}

	//	EndGame(false);
	//}

	//for (AShooterGamePlayerController* PlayerController : TActorRange<AShooterGamePlayerController>(GetWorld()))
	//{
	//	if (!PlayerController->IsDead())
	//	{
	//		return;
	//	}

	//	EndGame(false);
	//}

	for (AShooterGameAIController* Controller : TActorRange<AShooterGameAIController>(GetWorld()))
	{
		if (!Controller->IsDead())
		{
			return;
		}
	}
	
	EndGame(true);
}

void AKillEmAllGameMode::EndGame(bool bIsPlayerWinner)
{
	for (AController* Controller : TActorRange<AController>(GetWorld()))
	{
		// false and true shenanigans with controller and winner

		bool bIsWinner = Controller->IsPlayerController() == bIsPlayerWinner;
		Controller->GameHasEnded(Controller->GetPawn(), bIsWinner);
	}
}

void AKillEmAllGameMode::SpawnEnemy()
{
	if (ActiveEnemySpawnPoints.Num() < 1) return;

	for (ASGEnemySpawnPoint* EnemySpawnPoint : ActiveEnemySpawnPoints)
	{
		FVector Location = EnemySpawnPoint->GetActorLocation();
		FRotator Rotation = EnemySpawnPoint->GetActorRotation();
		TSubclassOf<ACharacter> SpawnClass = EnemySpawnPoint->EnemyClassToSpawn;
		ACharacter* SpawnedEnemy = GetWorld()->SpawnActor<ACharacter>(SpawnClass, Location, Rotation);
		//SpawnedEnemy->GetController()->Possess(SpawnedEnemy);
	}
}

void AKillEmAllGameMode::TempOnClientsReady()
{
	for (APlayerState* PState : GetGameState<AGameStateBase>()->PlayerArray)
	{
		if (ASGPlayerState* ShooterPState = Cast<ASGPlayerState>(PState))
		{
			ShooterPState->Client_OnClientsReady();
		}
	}
}

void AKillEmAllGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	RestartPlayer(NewPlayer);
}
