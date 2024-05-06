// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Shoot.h"
#include "AIController.h"
#include "EnemyRanged.h"

UBTTask_Shoot::UBTTask_Shoot()
{
	NodeName = TEXT("Shoot");
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	EnemyRanged->Shoot();

	return EBTNodeResult::Succeeded;
}
