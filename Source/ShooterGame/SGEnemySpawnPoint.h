// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "SGEnemySpawnPoint.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API ASGEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:

	ASGEnemySpawnPoint();

protected:

	//// ADD TRIGGER CLASS !!!////
	
	//UPROPERTY(EditAnywhere, Category = "Settings")
	//class AEnemyTrigger* LinkedTrigger;

	bool bIsActive;

	virtual void BeginPlay() override;

public:

	//class AEnemyTrigger* GetLinkedTrigger();

	bool IsActive();
	void ActivateSpawnPoint();

	UPROPERTY(EditAnywhere, Category = "Settings")
	class TSubclassOf<ACharacter> EnemyClassToSpawn = nullptr;
};
