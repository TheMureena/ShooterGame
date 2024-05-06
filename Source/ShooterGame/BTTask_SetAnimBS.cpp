// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SetAnimBS.h"
#include "AIController.h"
#include "EnemyRanged.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetAnimBS::UBTTask_SetAnimBS()
{
	NodeName = TEXT("SetAnimBS");
}

EBTNodeResult::Type UBTTask_SetAnimBS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyRanged* EnemyRanged = Cast<AEnemyRanged>(OwnerComp.GetAIOwner()->GetPawn());
	if (EnemyRanged == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	//FName PlayerKey = "Player";
	bool bHasSeenPlayer = OwnerComp.GetBlackboardComponent()->HasValidAsset();

	//UE_LOG(LogTemp, Warning, TEXT("Has seen Player is %s"), bHasSeenPlayer ? TEXT("true") : TEXT("false"));

	if (bHasSeenPlayer) 
	{
		EnemyRanged->bHasSeenPlayer = true;
	}
	else
	{
		EnemyRanged->bHasSeenPlayer = false;
	}

	return EBTNodeResult::Succeeded;
}
