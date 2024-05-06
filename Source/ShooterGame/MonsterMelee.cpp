// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterMelee.h"
#include "ShooterGameGameModeBase.h"
#include "ShooterCharacter.h"
#include "SGPlayerState.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Animation/AnimInstance.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMonsterMelee::AMonsterMelee()
{ 
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Pawn so these nee to be created
	GetCapsuleComponent()->InitCapsuleSize(44.f, 22.f);

	DamageCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Damage Collision"));
	DamageCollision->SetupAttachment(GetMesh(), TEXT("hand_RSocket"));
	//DamageCollision->GetCollisionEnabled();
	MeleeAnimMontage = nullptr;

	/// for the melee collision box
	CanDealDamage = false;

	SetReplicateMovement(true);
	bReplicates = true;
}

// Called when the game starts or when spawned
void AMonsterMelee::BeginPlay()
{
	Super::BeginPlay();
	
	Health = MaxHealth;

	if (HasAuthority())
	{
		AnimInstance = GetAnimInstMonster();

		DamageCollision->OnComponentBeginOverlap.AddDynamic(this,
			&AMonsterMelee::OnDealDamageOverlapBegin);
	}
}

void AMonsterMelee::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMonsterMelee, bIsRagdoll);
	DOREPLIFETIME(AMonsterMelee, Health);
}

void AMonsterMelee::MeleeAttack()
{
	if (UAnimInstance* AnimInstMonster = GetMesh()->GetAnimInstance())
	{
		if (MeleeAnimMontage)
		{
			AnimInstMonster->Montage_Play(MeleeAnimMontage);
		}
	}
}

void AMonsterMelee::OnDealDamageOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult) {

	FHitResult Hit;
	FVector AttackDirection;

	bInMeleeRange = MeleeRangeTrace(Hit, AttackDirection);
//	UE_LOG(LogTemp, Warning, TEXT("The boolean value is %s"), (bInMeleeRange ? TEXT("true") : TEXT("false")));
	if (bInMeleeRange)
	{
		AActor* HitActor = OtherActor;
		PlayerREF = Cast<AShooterCharacter>(OtherActor);
		//DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//DamageCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
		//DrawDebugPoint(GetWorld(), Hit.Location, 2, FColor::Red, true);
		if (HitActor != nullptr)
		{
			if (PlayerREF && CanDealDamage)
			{
				// hit actor can be damaged
				// deal damage to player
				UE_LOG(LogTemp, Warning, TEXT("Player Damaged"));
				FPointDamageEvent DamageEvent(MeleeDamage, SweepResult, AttackDirection, nullptr);

				HitActor->TakeDamage(MeleeDamage, DamageEvent, Controller, this);
				
				
			}
			else
			{
				// hit something else than player
				UE_LOG(LogTemp, Display, TEXT("Hit:%s"), *HitActor->GetName());
			}
		}
	}
}

void AMonsterMelee::SubtractHealth(int32 Damage)
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

void AMonsterMelee::Die()
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

float AMonsterMelee::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	SubtractHealth(DamageToApply);

	UE_LOG(LogTemp, Warning, TEXT("Health left: %f"), Health);

	if (IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Killed Enemy"), *EventInstigator->GetPawn()->GetName());
		AShooterCharacter* DamageCauserPlayer = Cast<AShooterCharacter>(EventInstigator->GetPawn());
		ASGPlayerState* PState = DamageCauserPlayer->GetPlayerState<ASGPlayerState>();

		if (PState)
		{
			PState->IncrementScore(KillScore);
		}
	}

	return DamageToApply;


}

bool AMonsterMelee::MeleeRangeTrace(FHitResult& Hit, FVector& AttackDirection)
{
	Controller = GetController();
	if (Controller == nullptr)
	{
		return false;
		UE_LOG(LogTemp, Warning, TEXT("NO CONTROLLER MONSTER"));
	}
	FVector Location;
	FRotator Rotation;

	AttackDirection = -Rotation.Vector();

	Controller->GetPlayerViewPoint(Location, Rotation);

	FVector End = (Location)+(Rotation.Vector() * MaxRange);

	//FCollisionObjectQueryParams ObjParams;
	//ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel2);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	// debugline draw
	// DrawDebugLine(GetWorld(), Location, End, FColor::Red, false, 2.0f, 0U , 1.f);
	return GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECC_GameTraceChannel1 ,Params);
}

bool AMonsterMelee::IsDead() const
{
	return Health <= 0;
}

void AMonsterMelee::OnRep_DeathMontage()
{
	//AnimInstance->StopAllMontages(0);
	SetReplicateMovement(false);
	//detach controller
	DetachFromControllerPendingDestroy();
	// disable capsule comp
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageCollision->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	SetActorEnableCollision(true);

	if (!bIsRagdoll)
	{
		// Ragdoll
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
		if (CharMovementComp)
		{
			CharMovementComp->StopMovementImmediately();
			CharMovementComp->DisableMovement();
			CharMovementComp->SetComponentTickEnabled(false);
		}
		if (HasAuthority())
		{
			SetLifeSpan(10.0f);
		}
		bIsRagdoll = true;
		ForceNetUpdate();
	}
}

UAnimInstance* AMonsterMelee::GetAnimInstMonster()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp == nullptr)
		return nullptr;
	UAnimInstance* ABP = MeshComp->GetAnimInstance();
	if (ABP == nullptr)
		return nullptr;
	return ABP;
}

