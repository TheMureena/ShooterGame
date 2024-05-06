// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplingHook.h"
#include "Components/StaticMeshComponent.h"
#include "CableComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Crosshair_Widget.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
	#include "Dbg.h"
#endif

// Sets default values
AGrapplingHook::AGrapplingHook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HookMesh"));
	HookMesh->SetupAttachment(Root);
	HookMesh->SetVisibility(false);
	
	// grappling cable setup
	GrappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("GrapplingCable"));
	GrappleCable->SetupAttachment(Root);
	GrappleCable->SetVisibility(false);

	bCanFire = true;
	DelayFire = 1.0f;
	DelayMissReturn = 3.0f;
	bIsAttached = false;

	// initialize a bunch of stuff
	ImpactAttenuation = nullptr;
	ImpactConcurrency = nullptr;
	ImpactDecals = nullptr;
	ImpactFX = nullptr;
	ImpactFXEnemy = nullptr;
	ImpactSFX = nullptr;
	ImpactSFXEnemy = nullptr;
	ShotSFX = nullptr;
	MissSFX = nullptr;
	ShootAttenuation = nullptr;
	ShootConcurrency = nullptr;
	MuzzleFlash = nullptr;

	bReplicates = true;

}

void AGrapplingHook::Fire()
{
	if (bCanFire)
	{
		// set shot delay timer to prevent spamming button
		GetWorld()->GetTimerManager().SetTimer(TGunFireDelayHandle, this, &AGrapplingHook::ControlFireDelay, DelayFire, false);

		FireGrapple();
		// for delay between shots
		bCanFire = false;

		#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
			Dbg->LOG_INFO_LOCATION(TEXT("Firing grappling hook"), this, GetActorLocation());
		#endif
	}
}

void AGrapplingHook::ControlFireDelay()
{
	bCanFire = true;
}

void AGrapplingHook::FireGrapple()
{
	// always spawn shotSFX sound even when not hitting anything
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShotSFX, GetActorLocation(), 1.0f, 1.0f, 0.0f, ShootAttenuation, ShootConcurrency);

	// play shoot animation on the grapple (maybe) skeletal mesh.
	//AnimationInstance->Montage_Play(ShootAnimMontage);

	//niagara fx stuff
	//UNiagaraComponent* MuzzleFlashComp = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, Mesh, TEXT("Muzzle"), FVector(0.f), MuzzleFlashRotationOffset, EAttachLocation::Type::KeepRelativeOffset, true);
	UNiagaraComponent* MuzzleFlashComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlash, GetActorLocation());
	//UNiagaraComponent* ShellEjectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShellEject, GetActorLocation());
	
	// probably set visible and use default end pos even when not hitting when shoot... but return somehow
	GrappleCable->SetVisibility(true);
	HookMesh->SetVisibility(true);

	FHitResult Hit;
	FVector ShotDirection;

	bool bSuccess = GrappleTrace(Hit, ShotDirection);
	if (bSuccess && !bIsAttached)
	{
		AActor* HitActor = Hit.GetActor();
		//DrawDebugPoint(GetWorld(), Hit.Location, 2, FColor::Red, true);
		if (HitActor != nullptr)
		{
			bIsAttached = true;
			GrabPoint = Hit.Location;

			#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
				Dbg->LOG_INFO_LOCATION(TEXT("Grappling hook Grab Point"), this, GrabPoint);
			#endif

			if (HitActor->CanBeDamaged())
			{
				// hit actor can be damaged
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSFXEnemy, Hit.Location, 1.0f, 1.0f, 0.0f, ImpactAttenuation, ImpactConcurrency);
				UNiagaraComponent* HitFXActor = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFXEnemy, Hit.Location, ShotDirection.Rotation(), FVector(3.0f));
				FPointDamageEvent DamageEvent(Damage, Hit, ShotDirection, nullptr);
				AController* OwnerController = GetOwnerController();
				HitActor->TakeDamage(Damage, DamageEvent, OwnerController, this);
			}
			else
			{
				// hit wall
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSFX, Hit.Location, 1.0f, 1.0f, 0.0f, ImpactAttenuation, ImpactConcurrency);
				UNiagaraComponent* HitFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, Hit.Location, ShotDirection.Rotation());
				// decal is using just a mat remember physical material for it to work
				UNiagaraComponent* HitDecal = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactDecalFX, Hit.Location, ShotDirection.Rotation());
			}
		}
	}
	else
	{
		// we didn't hit anything so return grapple

		// GrabPoint is set to trace end in GrappleTrace
		// thisFN->ReturnCable->ReleaseGrapple
		GrappleCable->EndLocation = GetActorTransform().InverseTransformPosition(GrabPoint);

		// rotate hook mesh to align with shot direction
		HookMesh->SetRelativeRotation(ShotDirection.Rotation());

		#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
			Dbg->LOG_INFO_LINE(TEXT("Grappling hook Missed,Grab Point"), this, GetActorLocation(), GrabPoint);
		#endif

		// set Miss delay timer to return grapple in xx seconds
		GetWorld()->GetTimerManager().SetTimer(TGrappleMissReturnHandle, this, &AGrapplingHook::ReturnCable, DelayMissReturn, false);
	}
}

void AGrapplingHook::ReleaseGrapple()
{
	GrappleCable->SetVisibility(false);
	GrappleCable->bEnableCollision = false;
	GrappleCable->bAttachEnd = true;
	CableReturning = false;
	bIsAttached = false;
	HookMesh->SetVisibility(false);
	// clear timer
	GetWorld()->GetTimerManager().ClearTimer(TGrappleMissReturnHandle);
}

void AGrapplingHook::ReturnCable()
{
	CableReturning = true;
	// set timer to hide cable
	GetWorld()->GetTimerManager().SetTimer(TGrappleMissReturnHandle, this, &AGrapplingHook:: ReleaseGrapple, DelayMissReturn, false);
}

bool AGrapplingHook::HasHit() const
{
	return bIsAttached;
}

bool AGrapplingHook::GrappleTrace(FHitResult& Hit, FVector& ShotDirection)
{
	AController* OwnerController = GetOwnerController();
	if (OwnerController == nullptr)
	{
		return false;
	}
	FVector Location;
	FRotator Rotation;

	OwnerController->GetPlayerViewPoint(Location, Rotation);
	ShotDirection = -Rotation.Vector();
	FVector End = (Location) + (Rotation.Vector() * MaxRange);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	// debugline draw
	//DrawDebugLine(GetWorld(), Location, End, FColor::Red, false, 20.f, 0U , 1.f);
	
	// set grabpoint to be end -> able to set cable endpoint in front of player in Fire miss else branch in case we miss
	GrabPoint = End;
	HookMesh->SetVisibility(true);

	return GetWorld()->LineTraceSingleByChannel(Hit, Location, End, ECollisionChannel::ECC_GameTraceChannel1, Params);
}



// Called when the game starts or when spawned
void AGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
}

void AGrapplingHook::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGrapplingHook, bIsAttached, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGrapplingHook, CableReturning, COND_SkipOwner);
}

bool AGrapplingHook::Server_Fire_Validate()
{
	return true;
}

void AGrapplingHook::Server_Fire_Implementation()
{
}

bool AGrapplingHook::Multicast_Fire_Validate()
{
	return true;
}

void AGrapplingHook::Multicast_Fire_Implementation()
{
}

// Called every frame
void AGrapplingHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAttached)
	{
		GrappleCable->EndLocation = GetActorTransform().InverseTransformPosition(GrabPoint);
		HookMesh->SetRelativeLocation(GrappleCable->EndLocation);
		OnGrappleDelegate.ExecuteIfBound(GrappleForce);
	}

	if (CableReturning)
	{
		FVector ReturnLocation = FMath::VInterpTo(GrappleCable->EndLocation, GetActorLocation(), DeltaTime, ReturnSpeed);
		GrappleCable->EndLocation = ReturnLocation;
		HookMesh->SetRelativeLocation(ReturnLocation);
	}
}

AController* AGrapplingHook::GetOwnerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return nullptr;
	return OwnerPawn->GetController();
}

#if ENABLE_VISUAL_LOG
void AGrapplingHook::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	IVisualLoggerDebugSnapshotInterface::GrabDebugSnapshot(Snapshot);
	const int32 CatIndex = Snapshot->Status.AddZeroed();
	FVisualLogStatusCategory& Category = Snapshot->Status[CatIndex];
	Category.Category = TEXT("GrapplingHook");
	Category.Add(TEXT("Grapple Is Attached"), FString::Printf(TEXT("%s"), bIsAttached ? TEXT("true") : TEXT("false")));
	Category.Add(TEXT("Grab Point"), FString::Printf(TEXT("%s"), *GrabPoint.ToString()));
	Category.Add(TEXT("Grapple End Location"), FString::Printf(TEXT("%s"), *GrappleCable->EndLocation.ToString()));
	Category.Add(TEXT("Range"), FString::Printf(TEXT("%f"), MaxRange));
	Category.Add(TEXT("CableReturnTimerRemaining"), FString::Printf(TEXT("%f"), GetWorld()->GetTimerManager().GetTimerRemaining(TGrappleMissReturnHandle)));

	//Snapshot->AddText("This is some text", "EnemyRanged", ELogVerbosity::Verbose);
	//Snapshot->AddText("More text", "EnemyRanged", ELogVerbosity::Warning);

	const float VisualScale = 50.0f;
	Snapshot->AddArrow(GetActorLocation(), GetVelocity(), "Velocity", ELogVerbosity::Verbose, FColor::Green, "Actor Velocity");
}
#endif