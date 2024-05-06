// Fill out your copyright notice in the Description page of Project Settings.


#include "SGEnemySpawnPoint.h"

ASGEnemySpawnPoint::ASGEnemySpawnPoint()
{
	bIsActive = true;
}

void ASGEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();
}

//AEnemyTrigger* ASGEnemySpawnPoint::GetLinkedTrigger()
//{
//	return LinkedTrigger;
//}

bool ASGEnemySpawnPoint::IsActive()
{
	return bIsActive;
}

void ASGEnemySpawnPoint::ActivateSpawnPoint()
{
	bIsActive = true;
}
