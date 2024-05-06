// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableBase.generated.h"

UCLASS()
class SHOOTERGAME_API AInteractableBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableBase();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ShooterGame Settings")
	FString UIMsg;

	UPROPERTY(EditAnywhere, Category = "ShooterGame Settings")
	FString ObjectName;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual FString GetUIMsg(class AShooterCharacter* Player);
	virtual void Use(class AShooterCharacter* Player);
};

