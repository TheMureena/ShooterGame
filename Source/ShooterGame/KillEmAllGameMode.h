// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameGameModeBase.h"
#include "KillEmAllGameMode.generated.h"

UCLASS()
class SHOOTERGAME_API AKillEmAllGameMode : public AShooterGameGameModeBase
{
	GENERATED_BODY()
	
public:

	AKillEmAllGameMode();

	virtual void PawnKilled(APawn* PawnKilled) override;

private:
	void EndGame(bool bIsPlayerWinner);

protected:
	TArray<class ASGEnemySpawnPoint*> ActiveEnemySpawnPoints;
	TArray<class ASGEnemySpawnPoint*> AllEnemySpawnPoints;

	TArray<class AShooterCharacter*> ConnectedPlayers;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<class AShooterCharacter> PlayerCharacterClass;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<class AEnemyRanged> EnemyRangedClass;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<class AMonsterMelee> MonsterMeleeClass;

protected:
	void SpawnEnemy();

	void TempOnClientsReady();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
