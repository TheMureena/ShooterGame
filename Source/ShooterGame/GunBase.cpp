// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
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
#include "EnemyRanged.h"

#if WITH_EDITOR
	#include "Dbg.h"
#endif

// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	
	WeaponName = "Default Name";

	MagazineSize = 30;
	TotalAmmoCount = 120;
	bCanFire = true;
	bIsFiring = false;
	DelayFire = 0.14f;

	// initialize a bunch of stuff
	//ImpactFX = nullptr;
	//ImpactFXEnemy = nullptr;
	//MuzzleFlash = nullptr;
	//ShellEject = nullptr;
	//ReloadAnimMontage = nullptr;
	//ShootAnimMontage = nullptr;
	//CharShootAnimMontage = nullptr;
	//CharReloadAnimMontage = nullptr;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	MagazineCurrent = MagazineSize;
}

void AGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGunBase, TotalAmmoCount, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGunBase, MagazineCurrent, COND_OwnerOnly);
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (SpreadAmount > 0)
	{
		SpreadAmount -= DeltaTime * 1;
		if (SpreadAmount < 0)
		{
			SpreadAmount = 0;
		}
		OnSpreadAmountChangedDelegate.ExecuteIfBound(SpreadAmount);
	}
}

int32 AGunBase::GetTotalAmmoCount() const
{
	return TotalAmmoCount;
}

int32 AGunBase::GetMagazineCurrent() const
{
	return MagazineCurrent;
}

int32 AGunBase::GetMagazineSize() const
{
	return MagazineSize;
}

FString AGunBase::GetGunName() const
{
	return WeaponName;
}

AController* AGunBase::GetOwnerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return nullptr;
	return OwnerPawn->GetController();
}

void AGunBase::Fire()
{
	if (AShooterCharacter* Player = Cast<AShooterCharacter>(GetOwner()))
	{
		if (Player->IsPlayingAction()) return;

		if (MagazineCurrent > 0 && bCanFire) 
		{
			if (UAnimInstance* AnimInstChar = Player->GetMesh()->GetAnimInstance())
			{
				if (CharShootAnimMontage)
				{
					// maybe add aiming ??

					AnimInstChar->Montage_Play(CharShootAnimMontage);
				}
			}

			bCanFire = false;
			bIsFiring = true;

			if (MagazineCurrent <= 0)
			{
				if (UAnimInstance* AnimInstChar = Player->GetMesh()->GetAnimInstance())
				{
					if (CharDryFireAnimMontage)
					{
						AnimInstChar->Montage_Play(CharDryFireAnimMontage);
					}
				}
			}
			else
			{
				if (UAnimInstance* AnimInstGun = GetMesh()->GetAnimInstance())
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

			--MagazineCurrent;

			FVector ShotDirection;
			TArray<FHitResult> HitResults = DoLineTrace(Player, ShotDirection);

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

			if (IsNetMode(ENetMode::NM_ListenServer))
			{
				if (HitResults.Num() > 0)
				{
					Multicast_Fire(HitResults[0], ShotDirection);
				}
				else
				{
					Multicast_Fire(FHitResult(), ShotDirection);
				}
			}
			else 
			{
				Server_Fire(HitResults, ShotDirection);
			}

			// set shot delay timer to prevent spamming button
			FTimerHandle TGunFireDelayHandle;
			GetWorld()->GetTimerManager().SetTimer(TGunFireDelayHandle, this, &AGunBase::ControlFireDelay, DelayFire, false);
		}
	}
}

void AGunBase::StopFiring()
{

}

bool AGunBase::Server_Fire_Validate(const TArray<FHitResult>& HitResults, FVector ShotDirection)
{
	return true;
}

void AGunBase::Server_Fire_Implementation(const TArray<FHitResult>& HitResults, FVector ShotDirection)
{
	if (MagazineCurrent > 0)
	{
		--MagazineCurrent;

		if (HitResults.Num() > 0)
		{
			for (FHitResult Result : HitResults)
			{
				if (AActor* HitActor = Result.GetActor())
				{
					if (HitActor->CanBeDamaged())
					{
						// hit actor can be damaged
						FPointDamageEvent DamageEvent(Damage, Result, Result.ImpactNormal, nullptr);
						AController* OwnerController = GetOwnerController();
						HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
					}
				}
			}
		}
		if (HitResults.Num() > 0)
			Multicast_Fire(HitResults[0], ShotDirection);
		else
			Multicast_Fire(FHitResult(), ShotDirection);
	}
}

bool AGunBase::Multicast_Fire_Validate(const FHitResult& HitResult, FVector ShotDirection)
{
	return true;
}

void AGunBase::Multicast_Fire_Implementation(const FHitResult& HitResult, FVector ShotDirection)
{
	if (AShooterCharacter* Player = Cast<AShooterCharacter>(GetOwner()))
	{
		if (!Player->IsLocallyControlled() && CharShootAnimMontage && ShootAnimMontage)
		{
			if (UAnimInstance* AnimInstChar = Player->GetMesh()->GetAnimInstance())
			{
				if (CharShootAnimMontage)
				{
					AnimInstChar->Montage_Play(CharShootAnimMontage);
				}
			}

			if (UAnimInstance* AnimInstGun = GetMesh()->GetAnimInstance())
			{
				if (ShootAnimMontage)
				{
					AnimInstGun->Montage_Play(ShootAnimMontage);
				}
			}

			if (AActor* HitActor = HitResult.GetActor())
			{
				if (HitActor->CanBeDamaged())
				{
					// hit actor can be damaged
					UNiagaraComponent* HitFXActor = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFXEnemy, HitResult.Location, ShotDirection.Rotation());
					FPointDamageEvent DamageEvent(Damage, HitResult, ShotDirection, nullptr);
					AController* OwnerController = GetOwnerController();
					HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
					UE_LOG(LogTemp, Warning, TEXT("Actor Name: %s"), *HitActor->GetName());
				}
				else
				{
					// hit wall
					UNiagaraComponent* HitFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, HitResult.Location, ShotDirection.Rotation());
					// decal is using just mat doesn't work now for some reason
					UNiagaraComponent* HitDecal = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactDecalFX, HitResult.Location, ShotDirection.Rotation());
				}
			}
		}
	}
}

bool AGunBase::Server_Reload_Validate()
{
	return true;
}

void AGunBase::Server_Reload_Implementation()
{
	Reload();
}

bool AGunBase::Multicast_Reload_Validate()
{
	return true;
}

void AGunBase::Multicast_Reload_Implementation()
{
	if (AShooterCharacter* Player = Cast<AShooterCharacter>(GetOwner()))
	{
		if (!Player->IsLocallyControlled() && ReloadAnimMontage)
		{
			if (UAnimInstance* AnimInstChar = Player->GetMesh()->GetAnimInstance())
			{
				if (CharReloadAnimMontage)
				{
					AnimInstChar->Montage_Play(CharReloadAnimMontage);
				}
			}

			if (UAnimInstance* AnimInstGun = GetMesh()->GetAnimInstance())
			{
				if (CharReloadAnimMontage)
				{
					AnimInstGun->Montage_Play(ReloadAnimMontage);
				}
			}
		}
	}
}

void AGunBase::Client_RefillAmmo_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("CLIENT REFILL AMMO"));
	RefillAmmo();
}

void AGunBase::RefillAmmo()
{
	if (AShooterCharacter* Player = Cast<AShooterCharacter>(GetOwner()))
	{
		TotalAmmoCount = 120;
		if (HasAuthority() && !Player->IsLocallyControlled())
			Client_RefillAmmo();
	}
}

TArray<FHitResult> AGunBase::DoLineTrace(AShooterCharacter* ShootingPlayer, FVector& ShotDirection)
{
	TArray<FHitResult> HitResults;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return HitResults;
	AController* OwnerController = OwnerPawn->GetController();
	if (OwnerController == nullptr)
		return HitResults;

	FVector Start;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Start, Rotation);


	// random vectors to offset bullets as in "recoil"
	RandomX = FMath::RandRange(-3.0, 3.0);
	RandomY = FMath::RandRange(-3.0, 3.0);

	Rotation += FRotator(RandomX, RandomY, 0) * SpreadAmount;
	// end recoil

	ShotDirection = -Rotation.Vector();

	FVector End = Start + Rotation.Vector() * MaxRange;

	SpreadAmount += RecoilAmount;
	if (SpreadAmount > 1)
	{
		SpreadAmount = 1;
	}

	OnSpreadAmountChangedDelegate.ExecuteIfBound(SpreadAmount);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ShootingPlayer);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 6.6f, 0, 2.0f);
	GetWorld()->LineTraceMultiByChannel(OUT HitResults, Start, End, ECollisionChannel::ECC_GameTraceChannel1, Params);

	return HitResults;
}

TArray<FHitResult> AGunBase::DoLineTrace(FVector MuzzleLocation, FRotator MuzzleRotation)
{
	// random vectors to offset bullets as in "recoil"
	RandomX = FMath::RandRange(-3.0, 3.0);
	RandomY = FMath::RandRange(-3.0, 3.0);

	MuzzleRotation += FRotator(RandomX, RandomY, 0) * SpreadAmount;

	FVector End = MuzzleLocation + MuzzleRotation.Vector() * MaxRange;

	SpreadAmount += RecoilAmount;
	if (SpreadAmount > 1)
	{
		SpreadAmount = 1;
	}

	OnSpreadAmountChangedDelegate.ExecuteIfBound(SpreadAmount);


	TArray<FHitResult> HitResults;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetOwner())
		Params.AddIgnoredActor(GetOwner());

	GetWorld()->LineTraceMultiByChannel(OUT HitResults, MuzzleLocation, End, ECollisionChannel::ECC_GameTraceChannel1, Params);

	return HitResults;
}

void AGunBase::ControlFireDelay()
{
	bCanFire = true;
	bIsFiring = false;
	if (AShooterCharacter* OwningPlayer = Cast<AShooterCharacter>(GetOwner()))
	{
		OwningPlayer->SetPlayingAction(false);
	}
}


void AGunBase::Reload()
{
	// check that you have totalammo left and that the magazine isn't full
	if (TotalAmmoCount > 0 && MagazineCurrent != MagazineSize)
	{
		if (AShooterCharacter* Player = Cast<AShooterCharacter>(GetOwner()))
		{
			if (Player->IsPlayingAction()) return;

			if (UAnimInstance* AnimInstChar = Player->GetMesh()->GetAnimInstance())
			{
				if (CharReloadAnimMontage)
				{
					bCanFire = false;
					AnimInstChar->Montage_Play(CharReloadAnimMontage);

					// timer for reload anim
					FTimerHandle ReloadRateHandle;
					GetWorld()->GetTimerManager().SetTimer(ReloadRateHandle, this, &AGunBase::ControlFireDelay, CharReloadAnimMontage->GetPlayLength(), false);
				}
			}
			if (APawn* Pawn = Cast<APawn>(GetOwner()))
			{
				if (Pawn->IsLocallyControlled())
				{
					if (UAnimInstance* AnimInstGun = GetGunMesh()->GetAnimInstance())
					{
						if (ReloadAnimMontage)
						{
							AnimInstGun->Montage_Play(ReloadAnimMontage);
						}
					}
				}
			}

			// calculate ammo stuff //

			if (TotalAmmoCount >= MagazineSize)
			{
				TotalAmmoCount -= (MagazineSize - MagazineCurrent);
				MagazineCurrent = MagazineSize;
			}
			else
			{
				if (MagazineCurrent + TotalAmmoCount > MagazineSize)
				{
					TotalAmmoCount -= (MagazineSize - MagazineCurrent);
					MagazineCurrent = MagazineSize;
				}
				else
				{
					MagazineCurrent += TotalAmmoCount;
					TotalAmmoCount = 0;
				}

			}

			if (TotalAmmoCount < 0)
			{
				TotalAmmoCount = 0;
			}

			Player->SetPlayingAction(true);

			if (IsNetMode(ENetMode::NM_ListenServer))
			{
				Multicast_Reload();
			}
			else
			{
				Server_Reload();
			}
		}
	}
}

void AGunBase::OnSpreadAmountChanged(float NewSpreadAmount)
{
	NewSpreadAmount = SpreadAmount;
}

USkeletalMeshComponent* AGunBase::GetGunMesh()
{
	return Mesh;
}
