// Fill out your copyright notice in the Description page of Project Settings.


#include "GunAuto.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Animation/AnimInstance.h"
#include "Crosshair_Widget.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

#include "ShooterGamePlayerController.h"
#include "ShooterCharacter.h"

#if WITH_EDITOR
	#include "Dbg.h"
#endif

// Sets default values
AGunAuto::AGunAuto()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MagazineSize = 30;
	TotalAmmoCount = 120;
	FireRate = 500;

	// initialize a bunch of stuff
	//EmptyShotSFX = nullptr;
	//ImpactAttenuation = nullptr;
	//ImpactConcurrency = nullptr;
	//ImpactDecals = nullptr;
	//ImpactFX = nullptr;
	//ImpactFXEnemy = nullptr;
	//ImpactSFX = nullptr;
	//ImpactSFXEnemy = nullptr;
	//ShotSFX = nullptr;
	//ReloadSFX = nullptr;
	//ShootAttenuation = nullptr;
	//ShootConcurrency = nullptr;
	//MuzzleFlash = nullptr;
	//ShellEject = nullptr;
	//ReloadAnimMontage = nullptr;
	//ShootAnimMontage = nullptr;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AGunAuto::BeginPlay()
{
	Super::BeginPlay();
	MagazineCurrent = MagazineSize;

	// calc fire rate to call shootauto at
	FireRate = 60 / FireRate;
}


void AGunAuto::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AGunAuto, bIsFiring, COND_SkipOwner);
}

void AGunAuto::Fire()
{
	if ( !bIsFiring && bCanFire )
	{

		if (AShooterCharacter* Player = Cast<AShooterCharacter>(GetOwner()))
			if (Player->IsPlayingAction()) return;

		bIsFiring = true;

		bCanFire = false;

		// set shot delay timer to prevent spamming button
		FTimerHandle TFireRateHandle;
		GetWorld()->GetTimerManager().SetTimer(TFireRateHandle, this, &AGunAuto::ControlFireDelay, DelayFire, false);

		OnClientFire();

		// Setup timer for automatic fire rate handling
		GetWorld()->GetTimerManager().SetTimer(GunFireHandle, this, &AGunAuto::OnClientFire, FireRate, true);

		//Dbg->LOG_INFO_LOCATION(TEXT("Start firing full auto"), this, GetActorLocation());

		if (!IsNetMode(ENetMode::NM_ListenServer))
		{
			Server_StartFullAutoFire(bIsFiring);
		}
	}
}

void AGunAuto::StopFiring()
{
	UE_LOG(LogTemp, Warning, TEXT("StopFiring"));
	GetWorld()->GetTimerManager().ClearTimer(GunFireHandle);

	bIsFiring = false;
	if (!IsNetMode(ENetMode::NM_ListenServer))
	{
		Server_StartFullAutoFire(bIsFiring);
	}
}


void AGunAuto::Server_Fire_Implementation(const TArray<FHitResult>& HitResults, FVector ShotDirection)
{
	if (MagazineCurrent > 0)
	{
		Super::Server_Fire_Implementation(HitResults, ShotDirection);

		if (HitResults.Num() > 0)
		{
			for (FHitResult Result : HitResults)
			{
				if (AActor* HitActor = Result.GetActor())
				{
					if (HitActor->CanBeDamaged())
					{
						// hit actor can be damaged
						FPointDamageEvent DamageEvent(Damage, Result, ShotDirection, nullptr);
						AController* OwnerController = GetOwnerController();
						HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
					}
				}
			}
		}
	}
	
}

bool AGunAuto::Server_StartFullAutoFire_Validate(bool IsFiring)
{
	return true;
}

void AGunAuto::Server_StartFullAutoFire_Implementation(bool IsFiring)
{
	UE_LOG(LogTemp, Warning, TEXT("Server_STARTFULLAUTOFIRE"));
	bIsFiring = IsFiring;
	OnRep_StartFullAutoFire();
}

void AGunAuto::OnRep_StartFullAutoFire()
{
	if (bIsFiring)
	{
		GetWorld()->GetTimerManager().SetTimer(GunFireHandle, this, &AGunAuto::PlayGunFX, FireRate, true);
		PlayGunFX();
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(GunFireHandle);
	}
}

void AGunAuto::PlayGunFX()
{
	UE_LOG(LogTemp, Warning, TEXT("ONREP PLAYING GUN FX FOR FULL AUTO"));
	if (AShooterCharacter* OwningCharacter = Cast<AShooterCharacter>(GetOwner()))
	{
		if (!OwningCharacter->IsLocallyControlled() && ShootAnimMontage)
		{
			if (UAnimInstance* CharAnimInstance = OwningCharacter->GetMesh()->GetAnimInstance())
			{
				if (CharShootAnimMontage)
				{
					//niagara fx stuff
					UNiagaraComponent* MuzzleFlashComp = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, Mesh, TEXT("Muzzle"), FVector(0.f), MuzzleFlashRotationOffset, EAttachLocation::Type::KeepRelativeOffset, true);
					UNiagaraComponent* ShellEjectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShellEject, Mesh->GetSocketLocation(TEXT("ShellEject")), ShellEjectRotationOffset);
					CharAnimInstance->Montage_Play(CharShootAnimMontage);
				}
			}

			if (UAnimInstance* AnimInstGun = GetGunMesh()->GetAnimInstance())
			{
				if (ShootAnimMontage)
				{
					AnimInstGun->Montage_Play(ShootAnimMontage);

					//niagara fx stuff
					UNiagaraComponent* MuzzleFlashComp = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, Mesh, TEXT("Muzzle"), FVector(0.f), MuzzleFlashRotationOffset, EAttachLocation::Type::KeepRelativeOffset, true);
					UNiagaraComponent* ShellEjectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShellEject, Mesh->GetSocketLocation(TEXT("ShellEject")), ShellEjectRotationOffset);
				}
			}
		}
	}
}

void AGunAuto::OnClientFire()
{
	if (MagazineCurrent > 0)
	{
		if (AShooterCharacter* ShootingPlayer = Cast<AShooterCharacter>(GetOwner()))
		{
			--MagazineCurrent;

			if (UAnimInstance* AnimInstChar = ShootingPlayer->GetMesh()->GetAnimInstance())
			{
				if (CharShootAnimMontage)
				{
					// maybe add aiming ??

					AnimInstChar->Montage_Play(CharShootAnimMontage);
				}
			}


			UE_LOG(LogTemp, Warning, TEXT("On Client Fire"));

			if (MagazineCurrent <= 0)
			{
				if (UAnimInstance* AnimInstChar = ShootingPlayer->GetMesh()->GetAnimInstance())
				{
					if (CharDryFireAnimMontage)
					{
						AnimInstChar->Montage_Play(CharDryFireAnimMontage);
					}
				}
			}
			else
			{
				if (UAnimInstance* AnimInstGun = GetGunMesh()->GetAnimInstance())
				{
					if (ShootAnimMontage)
					{
						AnimInstGun->Montage_Play(ShootAnimMontage);

						//niagara fx stuff
						UNiagaraComponent* MuzzleFlashComp = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, Mesh, TEXT("Muzzle"), FVector(0.f), MuzzleFlashRotationOffset, EAttachLocation::Type::KeepRelativeOffset, true);
						UNiagaraComponent* ShellEjectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShellEject, Mesh->GetSocketLocation(TEXT("ShellEject")), ShellEjectRotationOffset);
					}
				}
			}

			FVector ShotDirection;
			TArray<FHitResult> HitResults = DoLineTrace(ShootingPlayer, ShotDirection);

			if (HitResults.Num() > 0)
			{
				for (FHitResult& Result : HitResults)
				{
					if (AActor* HitActor = Result.GetActor())
					{
						if (HitActor->CanBeDamaged())
						{
							// hit actor can be damaged
							UNiagaraComponent* HitFXActor = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFXEnemy, Result.Location, ShotDirection.Rotation());
							FPointDamageEvent DamageEvent(Damage, Result, ShotDirection, nullptr);
							AController* OwnerController = GetOwnerController();
							HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
							UE_LOG(LogTemp, Warning, TEXT("Actor Name: %s"), *HitActor->GetName());
						}
						else
						{
							// hit wall
							UNiagaraComponent* HitFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, Result.Location, ShotDirection.Rotation());
							// decal is using just mat doesn't work now for some reason
							UNiagaraComponent* HitDecal = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactDecalFX, Result.Location, ShotDirection.Rotation());
						}
					}
				}
			}

			if (!IsNetMode(ENetMode::NM_ListenServer))
			{
				Server_Fire(HitResults, ShotDirection);
			}
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(GunFireHandle);
		bIsFiring = false;
		
		if (!IsNetMode(ENetMode::NM_ListenServer))
		{
			Server_StartFullAutoFire(bIsFiring);
		}
	}
}

float AGunAuto::GetFireRate() const
{
	return FireRate;
}

//bool AGunAuto::GunTrace(FHitResult &Hit, FVector &ShotDirection)
//{
//	AController *OwnerController = GetOwnerController();
//	if (OwnerController == nullptr)
//	{
//		return false;
//	}
//	FVector Location;
//	FRotator Rotation;
//
//	// random vectors to offset bullets as in "recoil"
//	RandomX = FMath::RandRange(-6.0, 6.0);
//	RandomY = FMath::RandRange(-6.0, 6.0);
//
//	OwnerController->GetPlayerViewPoint(Location, Rotation);
//	Rotation += FRotator(RandomX, RandomY, 0) * SpreadAmount;
//	ShotDirection = -Rotation.Vector();
//
//	FVector End = (Location) + (Rotation.Vector() * MaxRange);
//
//
//	SpreadAmount += RecoilAmount;
//	if (SpreadAmount > 1)
//	{
//		SpreadAmount = 1;
//	}
//
//	//Dbg->LOG_INFO_LOCATION(FString::SanitizeFloat(SpreadAmount), this, GetActorLocation());
//	//UE_LOG(LogTemp, Display, TEXT("spreadAmount: %f"), SpreadAmount);
//	OnSpreadAmountChangedDelegate.ExecuteIfBound(SpreadAmount);
//
//	//CrosshairWidget->HandleCrosshairScale(SpreadAmount);
//
//	FCollisionQueryParams Params;
//	Params.AddIgnoredActor(this);
//	if (GetOwner())
//		Params.AddIgnoredActor(GetOwner());
//	// debugline draw
//	//DrawDebugLine(GetWorld(), Location, End, FColor::Red, false, 20.f, 0U , 1.f);
//	return GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECollisionChannel::ECC_GameTraceChannel1, Params);
//}
