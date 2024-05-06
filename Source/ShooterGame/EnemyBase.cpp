// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "ShooterGameGameModeBase.h"
#include "ShooterCharacter.h"
#include "SGPlayerState.h"
#include "GunBase.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	Health = 100;
	bIsDead = false;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		Health = MaxHealth;
	}
}

void AEnemyBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyBase, bIsDead);
	DOREPLIFETIME(AEnemyBase, Health);
}

void AEnemyBase::SubtractHealth(int32 Damage)
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

void AEnemyBase::Die()
{
	if (HasAuthority())
	{
		if (AShooterGameGameModeBase* GM = GetWorld()->GetAuthGameMode<AShooterGameGameModeBase>())
		{
			GM->PawnKilled(this);
		}
		bIsDead = true;
		OnRep_Die();
	}
}

void AEnemyBase::OnRep_Die()
{
	SetCanBeDamaged(false);
	SetReplicateMovement(false);
	// disable capsule comp
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	SetActorEnableCollision(true);

	if (!bIsRagdoll)
	{
		// Ragdoll body
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		if (HasAuthority())
		{
			//detach controller
			DetachFromControllerPendingDestroy();
			SetLifeSpan(10.0f);
		}
		bIsRagdoll = true;
		ForceNetUpdate();
	}
}

void AEnemyBase::Hit(AShooterCharacter* Player, FHitResult HitResult)
{
	if (Player && !bIsDead)
	{
		if (ASGPlayerState* PState = Player->GetPlayerState<ASGPlayerState>())
		{
			if (AGunBase* PlayerGun = Player->GetGun())
			{
				SubtractHealth(PlayerGun->Damage);
				UE_LOG(LogTemp, Warning, TEXT("Hit Function On Enemy: %f"), Health);
				if (bIsDead)
				{
					PState->IncrementScore(KillScore);
				}
			}
		}
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	SubtractHealth(DamageToApply);

	UE_LOG(LogTemp, Warning, TEXT("Health left: %f"), Health);

	if (UAnimInstance* AnimInstEnemy = GetMesh()->GetAnimInstance())
	{
		if (HitReactAnimMontage)
		{
			AnimInstEnemy->Montage_Play(HitReactAnimMontage);
		}
	}
	
	if (bIsDead)
	{
		AShooterCharacter* DamageCauserPlayer = Cast<AShooterCharacter>(EventInstigator->GetPawn());
		ASGPlayerState* PState = DamageCauserPlayer->GetPlayerState<ASGPlayerState>();

		if (PState)
		{
			PState->IncrementScore(KillScore);
		}
	}

	return DamageToApply;
}


