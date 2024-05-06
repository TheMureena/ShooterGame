// Fill out your copyright notice in the Description page of Project Settings.


#include "GunSingle.h"
#include "ShooterCharacter.h"
#include "EnemyRanged.h"

#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

AGunSingle::AGunSingle()
{
	WeaponName = "Default Name";
	MagazineSize = 30;
	TotalAmmoCount = 120;
}

void AGunSingle::BeginPlay()
{
	Super::BeginPlay();
	MagazineCurrent = MagazineSize;
}
