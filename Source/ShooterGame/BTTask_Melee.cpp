// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Melee.h"
#include "AIController.h"
#include "MonsterMelee.h"

UBTTask_Melee::UBTTask_Melee()
{
	NodeName = TEXT("Melee");
}

EBTNodeResult::Type UBTTask_Melee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AMonsterMelee* Monster = Cast<AMonsterMelee>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Monster->MeleeAttack();

	return EBTNodeResult::Succeeded;
}
