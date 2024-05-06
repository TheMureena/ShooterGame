// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableBase.h"
#include "ShooterCharacter.h"


AInteractableBase::AInteractableBase()
{
	ObjectName = "Default";
	UIMsg = "Press F to use";

	bReplicates = true;
}

void AInteractableBase::BeginPlay()
{
	Super::BeginPlay();

	UIMsg.Append(FString(" " + ObjectName));
}

FString AInteractableBase::GetUIMsg(AShooterCharacter* Player)
{
	return UIMsg;
}

void AInteractableBase::Use(AShooterCharacter* Player)
{

}
