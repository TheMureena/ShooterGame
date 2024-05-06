// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "ShooterGameAIController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AShooterGameAIController : public AAIController
{
	GENERATED_BODY()

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

public:

	virtual void Tick(float DeltaTime) override;
	bool IsDead() const;

protected:

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* NewPawn) override;
private:

	UPROPERTY(EditAnywhere)
	class UBehaviorTree* AIBehavior;

	UPROPERTY(EditAnywhere)
	class UAIPerceptionComponent* PerceptionComp;
};
