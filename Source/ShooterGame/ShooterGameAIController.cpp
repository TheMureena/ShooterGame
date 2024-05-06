// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"
#include "ShooterCharacter.h"
#include "EnemyRanged.h"
#include "MonsterMelee.h"
#include "GunBase.h"
#include "VisualLogger/VisualLoggerTypes.h"

void AShooterGameAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterGameAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	///// this is here instead of begin play bc spawed enemies creation order !!!/////

	if (AIBehavior != nullptr)
	{
		RunBehaviorTree(AIBehavior);

		//APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		//GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"), PlayerPawn->GetActorLocation());

		GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), GetPawn()->GetActorLocation());

		// get random reachable loacation on navmesh in 800 radius
		UNavigationSystemV1* NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
		FNavLocation NavLocation;
		NavSystem->GetRandomReachablePointInRadius(GetPawn()->GetActorLocation(), 800.0f, NavLocation);
		FVector PatrolLocation = NavLocation.Location;
		GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolLocation"), PatrolLocation);
		//FString PatrolLocationString = PatrolLocation.ToString();
		//UE_LOG(LogTemp, Warning, TEXT("Patrolling to: %s"), *PatrolLocationString);
	}
}

void AShooterGameAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

bool AShooterGameAIController::IsDead() const
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetPawn());
	if (ControlledCharacter != nullptr)
	{
		return ControlledCharacter->IsDead();
	}

	AEnemyRanged* ControlledChar = Cast<AEnemyRanged>(GetPawn());
	if (ControlledChar != nullptr)
	{
		return ControlledChar->IsDead();
	}

	AMonsterMelee* ControlledMonster = Cast<AMonsterMelee>(GetPawn());
	if (ControlledMonster != nullptr)
	{
		return ControlledMonster->IsDead();
	}

	return true;
}

#if ENABLE_VISUAL_LOG
void AShooterGameAIController::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	IVisualLoggerDebugSnapshotInterface::GrabDebugSnapshot(Snapshot);
	const int32 CatIndex = Snapshot->Status.AddZeroed();
	FVisualLogStatusCategory& Category = Snapshot->Status[CatIndex];
	Category.Category = TEXT("AI");
}
#endif
