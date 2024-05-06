// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyRanged.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GunBase.h"
#include "ShooterGameGameModeBase.h"
#include "SGPlayerState.h"
#include "ShooterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "VisualLogger/VisualLoggerTypes.h"
#include "Net/UnrealNetwork.h"

AEnemyRanged::AEnemyRanged()
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.

	// Configure character movement
	UCharacterMovementComponent* MoveComp = CastChecked<UCharacterMovementComponent>(GetCharacterMovement());
	MoveComp->GravityScale = 1.0f;
	MoveComp->MaxAcceleration = 2400.0f;
	MoveComp->BrakingFrictionFactor = 1.0f;
	MoveComp->BrakingFriction = 6.0f;
	MoveComp->GroundFriction = 8.0f;
	MoveComp->BrakingDecelerationWalking = 1400.0f;
	MoveComp->bUseControllerDesiredRotation = false;
	MoveComp->bOrientRotationToMovement = false;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	MoveComp->MaxWalkSpeed = 200.0f;

	bHasSeenPlayer = false;
	bNeedsToReload = false;

	Gun = nullptr;
	ReloadAnimMontage = nullptr;
	ShootAnimMontage = nullptr;
	DryFireAnimMontage = nullptr;

	SetReplicateMovement(true);
	bReplicates = true;
}

void AEnemyRanged::BeginPlay()
{
	Super::BeginPlay();


	if (HasAuthority())
	{
		Health = MaxHealth;

		AnimationInstance = GetAnimInstEnemyRanged();

		GetMesh()->HideBoneByName(TEXT("weapon_r"), EPhysBodyOp::PBO_None);

		Gun = GetWorld()->SpawnActor<AGunBase>(DefaultGunClass);
		Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("WeaponSocket"));
		Gun->SetOwner(this);

		// set infinite ammo
		Gun->TotalAmmoCount = 30000;
	}
	
}

void AEnemyRanged::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyRanged, bIsRagdoll);
	DOREPLIFETIME(AEnemyRanged, Health);
	DOREPLIFETIME(AEnemyRanged, Gun);
}

void AEnemyRanged::SubtractHealth(int32 Damage)
{
	if (HasAuthority())
	{
		Health -= Damage;
		if (Health <= 0)
		{
			Die();
		}
	}
}

void AEnemyRanged::Die()
{
	if (HasAuthority())
	{
		if (AShooterGameGameModeBase* GM = GetWorld()->GetAuthGameMode<AShooterGameGameModeBase>())
		{
			GM->PawnKilled(this);
		}

		OnRep_DeathMontage();
	}
}

float AEnemyRanged::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	SubtractHealth(DamageToApply);

	if (GetAnimInstEnemyRanged())
		GetAnimInstEnemyRanged()->Montage_Play(HitReactAnimMontage);
	//UE_LOG(LogTemp, Warning, TEXT("Health left: %f"), Health);

	if (IsDead())
	{
		AShooterCharacter* DamageCauserPlayer = Cast<AShooterCharacter>(EventInstigator->GetPawn());
		ASGPlayerState* PState = DamageCauserPlayer->GetPlayerState<ASGPlayerState>();

		if (PState)
		{
			PState->IncrementScore(KillScore);
		}

		if (HasAuthority())
		{
			AShooterGameGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AShooterGameGameModeBase>();
			if (GameMode != nullptr)
			{
				GameMode->PawnKilled(this);
			}

			OnRep_DeathMontage();
		}

		return 0;
	}

	return DamageToApply;
}

bool AEnemyRanged::IsDead() const
{
	return Health <= 0;
}

bool AEnemyRanged::HasSeenPlayer() const
{
	return bHasSeenPlayer;
}

void AEnemyRanged::OnRep_DeathMontage()
{
	SetCanBeDamaged(false);
	Gun->StopFiring();
	Gun->GetGunMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	Gun->SetActorEnableCollision(true);
	SetReplicateMovement(false);
	// disable capsule comp
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	SetActorEnableCollision(true);

	if (!bIsRagdoll)
	{
		// Ragdoll gun
		Gun->GetGunMesh()->SetAllBodiesSimulatePhysics(true);
		Gun->GetGunMesh()->SetSimulatePhysics(true);
		Gun->GetGunMesh()->WakeAllRigidBodies();
		Gun->GetGunMesh()->bBlendPhysics = true;

		// Ragdoll body
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		if (HasAuthority())
		{
			//detach controller
			Gun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			DetachFromControllerPendingDestroy();
			SetLifeSpan(10.0f);
			Gun->SetLifeSpan(10.0f);

		}
		bIsRagdoll = true;
		ForceNetUpdate();
	}
}

void AEnemyRanged::Shoot()
{
	if (!bNeedsToReload)
	{
		if (Gun->GetMagazineCurrent() > 0)
		{
			if (AnimationInstance)
				AnimationInstance->Montage_Play(ShootAnimMontage);
			Gun->Fire();
		}
		else 
		{
			if (!bNeedsToReload)
			{
				bNeedsToReload = true;
				AnimationInstance->Montage_Play(DryFireAnimMontage);
				Gun->StopFiring();
				// Force AI to reload when no ammo left
				//Reload();
			}
		}
	}
}

void AEnemyRanged::StopShooting()
{
	Gun->StopFiring();
}

void AEnemyRanged::Reload()
{
	if (AnimationInstance->Montage_IsPlaying(ReloadAnimMontage) == false && bNeedsToReload)
	{
		AnimationInstance->Montage_Play(ReloadAnimMontage);
		Gun->Reload();
		bNeedsToReload = false;
	}
}

void AEnemyRanged::Melee()
{

}

UAnimInstance* AEnemyRanged::GetAnimInstEnemyRanged()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp == nullptr)
		return nullptr;
	UAnimInstance* ABP = MeshComp->GetAnimInstance();
	if (ABP == nullptr)
		return nullptr;
	return ABP;
}

#if ENABLE_VISUAL_LOG
void AEnemyRanged::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	IVisualLoggerDebugSnapshotInterface::GrabDebugSnapshot(Snapshot);
	const int32 CatIndex = Snapshot->Status.AddZeroed();
	FVisualLogStatusCategory& Category = Snapshot->Status[CatIndex];
	Category.Category = TEXT("EnemyRanged");
	Category.Add(TEXT("Health"), FString::Printf(TEXT("%f"), Health));
	Category.Add(TEXT("Has Seen Player"), FString::Printf(TEXT("%s"), bHasSeenPlayer ? TEXT("true") : TEXT("false")));
	Category.Add(TEXT("Needs To Reload"), FString::Printf(TEXT("%s"), bNeedsToReload ? TEXT("true") : TEXT("false")));

	//Snapshot->AddText("This is some text", "EnemyRanged", ELogVerbosity::Verbose);
	//Snapshot->AddText("More text", "EnemyRanged", ELogVerbosity::Warning);

	const float VisualScale = 50.0f;
	Snapshot->AddArrow(GetActorLocation(), GetVelocity(), "Velocity", ELogVerbosity::Verbose, FColor::Green, "Actor Velocity");
}
#endif