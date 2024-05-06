// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StopShooting.h"
#include "AIController.h"
#include "EnemyRanged.h"

UBTTask_StopShooting::UBTTask_StopShooting()
{
	NodeName = TEXT("StopShooting");
}

EBTNodeResult::Type UBTTask_StopShooting::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	EnemyRanged->StopShooting();

	return EBTNodeResult::Succeeded;
}
