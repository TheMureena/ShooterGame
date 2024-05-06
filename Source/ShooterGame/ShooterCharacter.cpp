// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraFunctionLibrary.h"
#include "InteractableBase.h"

#include "NiagaraComponent.h"
#include "GunBase.h"
#include "GrapplingHook.h"
#include "ShooterGameGameModeBase.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedPlayerInput.h"
#include "Animation/AnimInstance.h"
#include "ShooterGameSaveGame.h"
#include "Crosshair_Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Net/UnrealNetwork.h"

#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
	#include "Dbg.h"
#endif

///////
///////

UENUM(BlueprintType)
enum class EGunSlot : uint8
{
	SLOT_1 = 0,
	SLOT_2 = 1,
	SLOT_3 = 2,
	SLOT_4 = 3,
	SLOT_5 = 4,
	SLOT_6 = 5
};

// set default values
AShooterCharacter::AShooterCharacter()
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
	MoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->SetCrouchedHalfHeight(25.0f);
	MoveComp->JumpZVelocity = 700.f;
	MoveComp->AirControl = 0.35f;
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MaxWalkSpeedCrouched = SlideSpeed;
	MoveComp->MinAnalogWalkSpeed = 20.f;

	// rotate only Yaw when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;


	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	// Create Camera Manager for shakes MikkoL
	CameraManager = CreateDefaultSubobject<APlayerCameraManager>(TEXT("CameraManager"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) are set in the derived blueprint asset named BP_ShooterCharacter (to avoid direct content references in C++)

	// set weapon to first
	ActiveGunIndex = 0;

	bIsRunning = false;
	bIsSliding = false;
	bIsGrappling = false;
	bIsPlayingAction = false;

	/// init a bunch of stuff
	CurrentGun = nullptr;
	SwitchWeaponAction = nullptr;
	ReloadAction = nullptr;
	ShootAction = nullptr;
	JumpAction = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	MeleeAction = nullptr;
	DefaultMappingContext = nullptr;
	SwitchWeaponAnimMontage = nullptr;
	MeleeAnimMontage = nullptr;
	bUseDefaultGunClass = false;

	Interactable = nullptr;
	InteractionRange = 150.0f;

	SetReplicateMovement(true);
	bReplicates = true;
	SetReplicates(true);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(TInteractTimerHandle, this, &AShooterCharacter::SetInteractableObject, 0.1f, true);

	if (HasAuthority())
	{
		Health = MaxHealth;
		AnimationInstance = GetAnimInstCharacter();

		GetMesh()->HideBoneByName(TEXT("weapon_r"), EPhysBodyOp::PBO_None);

		GrapplingHook = GetWorld()->SpawnActor<AGrapplingHook>(GrapplingHookClass);
		GrapplingHook->SetOwner(this);
		GrapplingHook->OnGrappleDelegate.BindUObject(this, &AShooterCharacter::AddGrappleForce);
		OnRep_AttachGrapple();

		//UE_LOG(LogTemp, Warning, TEXT("%f"), AllGunsEquipped.Num());

		if (GunClasses.Num() >= 1 && bUseDefaultGunClass == false)
		{
			// Uses Array of guns

			for (TSubclassOf<AGunBase>& GunClass : GunClasses)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s"), *GunClass->GetFName().ToString());
				CurrentGun = GetWorld()->SpawnActor<AGunBase>(GunClass);
				CurrentGun->SetOwner(this);
				PreviousGun = CurrentGun;
				CurrentGun->SetActorHiddenInGame(true);
				AllGunsEquipped.Add(CurrentGun);
				OnRep_AttachGun();
			}

			// unhide the first CurrentGun
			CurrentGun = AllGunsEquipped[ActiveGunIndex];
			PreviousGun = CurrentGun;
			CurrentGun->SetActorHiddenInGame(false);
			OnRep_AttachGun();
			//UE_LOG(LogTemp, Warning, TEXT("Guns Equipped: %i"), AllGunsEquipped.Num());
		}
		else
		{
			// uses default CurrentGun

			CurrentGun = GetWorld()->SpawnActor<AGunBase>(DefaultGunClass);
			CurrentGun->SetOwner(this);
			AllGunsEquipped.Add(CurrentGun);
			OnRep_AttachGun();
		}
	}

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}

////////////////////////////////////////////
// Called to bind functionality to input //

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Crouching
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Ongoing, this, &AShooterCharacter::StartSlide);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopSlide);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);

		//Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Shoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopShooting);

		//Reloading
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Reload);

		//Switching Weapons
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchGun);
		
		//Switching Weapons with nums 1-6 atm
		EnhancedInputComponent->BindAction(GunSlot1, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchWeaponSlot1);
		EnhancedInputComponent->BindAction(GunSlot2, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchWeaponSlot2);
		EnhancedInputComponent->BindAction(GunSlot3, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchWeaponSlot3);
		EnhancedInputComponent->BindAction(GunSlot4, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchWeaponSlot4);
		EnhancedInputComponent->BindAction(GunSlot5, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchWeaponSlot5);
		EnhancedInputComponent->BindAction(GunSlot6, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchWeaponSlot6);

		//Meleeing
		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Melee);

		//Running // could theoretically use this type of logic for isshooting and other stuff on anim state machines!!
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Ongoing, this, &AShooterCharacter::Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopRunning);

		//Grappling
		EnhancedInputComponent->BindAction(GrapplingHookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::ToggleGrapple);
		//EnhancedInputComponent->BindAction(GrapplingHookAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopGrappling);

		// Use
		EnhancedInputComponent->BindAction(UseAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Interact);

		// Saving
	//EnhancedInputComponent->BindAction(SaveGameAction, ETriggerEvent::Triggered, this, &UShooterGameSaveGame::SaveGame);
	}
}

void AShooterCharacter::OnRep_AttachGun()
{
	if (PreviousGun)
	{
		UE_LOG(LogTemp, Warning, TEXT("HIDING PREVIOUS GUN"));
		PreviousGun->SetActorHiddenInGame(true);
	}
	if (CurrentGun)
	{
		CurrentGun->SetActorHiddenInGame(false);
		PreviousGun = CurrentGun;
		CurrentGun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));	
	}
}

void AShooterCharacter::OnRep_AttachGrapple()
{
	if (GrapplingHook)
	{
		GrapplingHook->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("GrappleSocket"));
	}
}


void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, CurrentGun);
	DOREPLIFETIME(AShooterCharacter, GrapplingHook);
	DOREPLIFETIME(AShooterCharacter, AllGunsEquipped);
	DOREPLIFETIME_CONDITION(AShooterCharacter, ActiveGunIndex, COND_OwnerOnly);

	DOREPLIFETIME(AShooterCharacter, bIsDead);
	DOREPLIFETIME(AShooterCharacter, Health);
	DOREPLIFETIME(AShooterCharacter, bIsGrappling);
	DOREPLIFETIME(AShooterCharacter, bIsRagdoll);
	DOREPLIFETIME(AShooterCharacter, bIsRunning);
	DOREPLIFETIME(AShooterCharacter, bIsSliding);
	DOREPLIFETIME(AShooterCharacter, TurnRate);
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
		float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
		DamageToApply = FMath::Min(Health, DamageToApply);
		SubtractHealth(DamageToApply);

		// avoid crash bc bots dont have this
		if (GetCameraManager() && IsNetMode(ENetMode::NM_Client))
		{
			GetCameraManager()->StartCameraShake(CS_HitReact);
		}

		if (GetAnimInstCharacter())
			GetAnimInstCharacter()->Montage_Play(HitReactAnimMontage);
		//UE_LOG(LogTemp, Warning, TEXT("Health left: %f"), Health);

		return DamageToApply;
}

bool AShooterCharacter::IsDead() const
{
	return Health <= 0;
}

bool AShooterCharacter::IsRunning() const
{
	return bIsRunning;
}

bool AShooterCharacter::IsSliding() const
{
	return bIsSliding;
}

bool AShooterCharacter::IsGrappling() const
{
	return bIsGrappling;
}

void AShooterCharacter::SetPlayingAction(bool NewIsPerformingAction)
{
	bIsPlayingAction = NewIsPerformingAction;
}

float AShooterCharacter::GetHealthPercentage() const
{
	return Health / MaxHealth;
}

void AShooterCharacter::OnRep_Die()
{
	SetCanBeDamaged(false);
	CurrentGun->StopFiring();
	CurrentGun->GetGunMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	CurrentGun->SetActorEnableCollision(true);

	SetReplicateMovement(false);
	//detach controller
	DetachFromControllerPendingDestroy();
	// disable capsule comp
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	SetActorEnableCollision(true);

	if (!bIsRagdoll)
	{
		// Ragdoll CurrentGun
		CurrentGun->GetGunMesh()->SetAllBodiesSimulatePhysics(true);
		CurrentGun->GetGunMesh()->SetSimulatePhysics(true);
		CurrentGun->GetGunMesh()->WakeAllRigidBodies();
		CurrentGun->GetGunMesh()->bBlendPhysics = true;

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
			//detach controller
			CurrentGun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			DetachFromControllerPendingDestroy();
			SetLifeSpan(10.0f);
			CurrentGun->SetLifeSpan(10.0f);
		}
		bIsRagdoll = true;
		ForceNetUpdate();
	}
}

void AShooterCharacter::PlayerDie()
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

void AShooterCharacter::RefreshInteractableObject()
{
	Interactable = nullptr;
}

void AShooterCharacter::Interact()
{
	if (Interactable)
	{
		Interactable->Use(this);

		if (!HasAuthority())
			Server_Interact(Interactable);

		Interactable = nullptr;
	}
}

bool AShooterCharacter::Server_Interact_Validate(AInteractableBase* InteractingObject)
{
	return true;
}

void AShooterCharacter::Server_Interact_Implementation(AInteractableBase* InteractingObject)
{
	float DistanceFromInteractable = GetDistanceTo(InteractingObject);

	if (DistanceFromInteractable < InteractionRange + 80.0f)
		InteractingObject->Use(this);
}

void AShooterCharacter::SetInteractableObject()
{
	FHitResult HitResult;

	AController* OwnerController = GetController();
	if (OwnerController == nullptr)
		return;

	FVector Start;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Start, Rotation);

	FVector End = Start + Rotation.Vector() * InteractionRange;

	FCollisionObjectQueryParams ObjParams;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 6.6f, 0, 2.0f);
	GetWorld()->LineTraceSingleByObjectType(OUT HitResult, Start, End, ObjParams, Params);

	AInteractableBase* Temp = Cast<AInteractableBase>(HitResult.GetActor());
	if (Interactable == nullptr && Temp)
	{
		Interactable = Temp;
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *Temp->GetName());
		NewInteractMsg.Broadcast(Interactable->GetUIMsg(this));
	}
	else if (Interactable && Temp == nullptr)
	{
		Interactable = nullptr;
		NewInteractMsg.Broadcast(FString());
	}
}

void AShooterCharacter::Shoot(const FInputActionValue& Value)
{
	if (CurrentGun && bIsRunning == false)
	{
		CurrentGun->Fire();
	}
}

void AShooterCharacter::StopShooting(const FInputActionValue& Value)
{
	if (CurrentGun)
	{
		CurrentGun->StopFiring();
	}
}

void AShooterCharacter::Reload(const FInputActionValue& Value)
{
	if (CurrentGun)
	{
		CurrentGun->Reload();
	}
}

void AShooterCharacter::SwitchGun(const FInputActionValue& Value)
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;

		//hide gun in previous index
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);

		// increment index
		if (ActiveGunIndex == AllGunsEquipped.Num() - 1 )
		{
			ActiveGunIndex = 0;
		}
		else
		{
			ActiveGunIndex++;
		}

		// unhide gun in next index and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);
		
		if (GetAnimInstCharacter())
			GetAnimInstCharacter()->Montage_Play(SwitchWeaponAnimMontage);

		Server_SwitchGun(CurrentGun, ActiveGunIndex);
	}
}

void AShooterCharacter::SwitchWeaponSlot1()
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;
		//hide current CurrentGun
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);
		ActiveGunIndex = 0;
		// unhide next CurrentGun and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);

		AnimationInstance->Montage_Play(SwitchWeaponAnimMontage);
	}
}

void AShooterCharacter::SwitchWeaponSlot2()
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;
		//hide current CurrentGun
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);
		ActiveGunIndex = 1;
		// unhide next CurrentGun and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);

		AnimationInstance->Montage_Play(SwitchWeaponAnimMontage);
	}
}

void AShooterCharacter::SwitchWeaponSlot3()
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;
		//hide current CurrentGun
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);
		ActiveGunIndex = 2;
		// unhide next CurrentGun and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);

		AnimationInstance->Montage_Play(SwitchWeaponAnimMontage);
	}
}

void AShooterCharacter::SwitchWeaponSlot4()
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;
		//hide current CurrentGun
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);
		ActiveGunIndex = 3;
		// unhide next CurrentGun and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);

		AnimationInstance->Montage_Play(SwitchWeaponAnimMontage);
	}
}

void AShooterCharacter::SwitchWeaponSlot5()
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;
		//hide current CurrentGun
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);
		ActiveGunIndex = 4;
		// unhide next CurrentGun and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);

		AnimationInstance->Montage_Play(SwitchWeaponAnimMontage);
	}
}

void AShooterCharacter::SwitchWeaponSlot6()
{
	if (CurrentGun)
	{
		if (CurrentGun->IsFiring()) return;
		//hide current CurrentGun
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(true);
		ActiveGunIndex = 5;
		// unhide next CurrentGun and set CurrentGun variable
		CurrentGun = AllGunsEquipped[ActiveGunIndex];
		AllGunsEquipped[ActiveGunIndex]->SetActorHiddenInGame(false);

		AnimationInstance->Montage_Play(SwitchWeaponAnimMontage);
	}
}

void AShooterCharacter::Melee(const FInputActionValue& Value)
{

	if (AnimationInstance->IsAnyMontagePlaying() == false && bIsRunning == false)
	{
		// todo melee hit detection
		// weapon bone, thats now hidden
		AnimationInstance->Montage_Play(MeleeAnimMontage);
	}
}

void AShooterCharacter::Run(const FInputActionValue& Value)
{
	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AShooterCharacter::StartSlide(const FInputActionValue& Value)
{
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());

	if (CharMovementComp->IsMovingOnGround() && bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("START SLIDE"));
		Crouch(true);
		bIsSliding = true;
	}

}

void AShooterCharacter::StopSlide(const FInputActionValue& Value)
{
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());

	if (bIsCrouched || CharMovementComp->bWantsToCrouch)
	{
		UE_LOG(LogTemp, Warning, TEXT("STOP SLIDE"));
		UnCrouch(true);
		bIsSliding = false;
	}
}

void AShooterCharacter::StopRunning(const FInputActionValue& Value)
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AShooterCharacter::ToggleGrapple()
{
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());

	if (bIsGrappling)
	{
		UE_LOG(LogTemp, Warning, TEXT("STOP GRAPPLING"));
		StopGrappling();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("START GRAPPLING"));
		StartGrappling();
	}
}

void AShooterCharacter::StartGrappling()
{
		GrapplingHook->Fire();

		if (GrapplingHook->bIsAttached)
		{
			bIsGrappling = true;

			UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
			if (CharMovementComp)
			{
				CharMovementComp->SetMovementMode(EMovementMode::MOVE_Flying);
			}
		}
}	

void AShooterCharacter::StopGrappling()
{
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharMovementComp->IsFlying())
	{
		bIsGrappling = false;
		GrapplingHook->ReturnCable();

		if (CharMovementComp)
		{
			if (!CharMovementComp->IsFalling())
				CharMovementComp->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}

void AShooterCharacter::AddGrappleForce(float GrappleForce)
{
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
	if (CharMovementComp)
		CharMovementComp->AddForce( ( (GrapplingHook->GrabPoint) - GetActorLocation() ).GetSafeNormal() * GrappleForce);
}

void AShooterCharacter::OnVelocityChanged(float NewVelocity)
{
	// this needs fixing
	NewVelocity = GetVelocity().Size() / 500;
}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		// check if grappling
		if (!bIsGrappling)
		{
			if (MovementVector.X != 0.0f)
			{
				const FVector MovementDirection = YawRotation.RotateVector(FVector::RightVector);
				AddMovementInput(MovementDirection, MovementVector.X);
			}

			if (MovementVector.Y != 0.0f)
			{
				const FVector MovementDirection = YawRotation.RotateVector(FVector::ForwardVector);
				AddMovementInput(MovementDirection, MovementVector.Y);
			}
		}
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// For TurnInPlace
	TurnRate = LookAxisVector;

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		if (LookAxisVector.X != 0.0f)
		{
			AddControllerYawInput(LookAxisVector.X);
		}

		if (LookAxisVector.Y != 0.0f)
		{
			// invert this to have it not inverted pitch
			AddControllerPitchInput(-LookAxisVector.Y);
		}
	}
}

void AShooterCharacter::SubtractHealth(int32 Damage)
{
	if (HasAuthority())
	{
		Health -= Damage;
		if (Health <= 0)
		{
			PlayerDie();
		}
	}
}

APlayerCameraManager* AShooterCharacter::GetCameraManager()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
		return nullptr;
	APlayerCameraManager* CM = PlayerController->PlayerCameraManager;
	if (CM == nullptr)
		return nullptr;
	return CM;
}

UAnimInstance* AShooterCharacter::GetAnimInstCharacter()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp == nullptr)
		return nullptr;
	UAnimInstance* ABP = MeshComp->GetAnimInstance();
	if (ABP == nullptr)
		return nullptr;
	return ABP;
}

AGunBase* AShooterCharacter::GetGun()
{
	AGunBase* GunActor = Cast<AGunBase>(CurrentGun);
	if (GunActor == nullptr)
		return nullptr;
	return CurrentGun;
}

void AShooterCharacter::GivePlayerNewGun(AGunBase* NewGun)
{
	if (HasAuthority() && NewGun)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADDING NEW CurrentGun TO CurrentGun ARRAY"));
		AllGunsEquipped.Add(NewGun);
		ActiveGunIndex = AllGunsEquipped.Num() - 1;

		CurrentGun = NewGun;
		CurrentGun->SetOwner(this);
		OnRep_AttachGun();
	}
}

bool AShooterCharacter::Server_SwitchGun_Validate(AGunBase* NewGun, int32 NewActiveGunIndex)
{
	return true;
}

void AShooterCharacter::Server_SwitchGun_Implementation(AGunBase* NewGun, int32 NewActiveGunIndex)
{
	ActiveGunIndex = NewActiveGunIndex;
	CurrentGun = NewGun;
	OnRep_AttachGun();
}

bool AShooterCharacter::Server_Grapple_Validate(AGunBase* NewGun, int32 NewActiveGunIndex)
{
	return true;
}

void AShooterCharacter::Server_Grapple_Implementation(AGunBase* NewGun, int32 NewActiveGunIndex)
{
}

AGrapplingHook* AShooterCharacter::GetGrapplingHook()
{
	AGrapplingHook* GrapplingHookActor = Cast<AGrapplingHook>(GrapplingHook);
	if (GrapplingHookActor == nullptr)
		return nullptr;
	return GrapplingHook;
}

#if ENABLE_VISUAL_LOG
void AShooterCharacter::GrabDebugSnapshot(FVisualLogEntry* Snapshot) const
{
	IVisualLoggerDebugSnapshotInterface::GrabDebugSnapshot(Snapshot);
	const int32 CatIndex = Snapshot->Status.AddZeroed();
	FVisualLogStatusCategory& Category = Snapshot->Status[CatIndex];
	Category.Category = TEXT("ShooterCharacter");
	Category.Add(TEXT("Is Grappling"), FString::Printf(TEXT("%s"), bIsGrappling ? TEXT("true") : TEXT("false")));
	Category.Add(TEXT("Velocity"), FString::Printf(TEXT("%s"), *GetVelocity().ToString()));
	//Category.Add(TEXT("Needs To Reload"), FString::Printf(TEXT("%d"), bNeedsToReload));

	//Snapshot->AddText("This is some text", "EnemyRanged", ELogVerbosity::Verbose);
	//Snapshot->AddText("More text", "EnemyRanged", ELogVerbosity::Warning);

	const float VisualScale = 50.0f;
	Snapshot->AddArrow(GetActorLocation(), GetVelocity(), "Velocity", ELogVerbosity::Verbose, FColor::Green, "Actor Velocity");
}
#endif
