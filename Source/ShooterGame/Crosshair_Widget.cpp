// Fill out your copyright notice in the Description page of Project Settings.


#include "Crosshair_Widget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Kismet/KismetMathLibrary.h"
#include "GunBase.h"
//#include "ShooterCharacter.h"
//#include "GameFramework/CharacterMovementComponent.h"

UCrosshair_Widget::UCrosshair_Widget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	TestVelocity = 0.0f;
	MaxVelocity = 1.0f;

	MaxOffset = 100.0f;

}

void UCrosshair_Widget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	SetCanvasPanelSlots();

	HandleCrosshairScale(TestVelocity);
}

void UCrosshair_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	SetCanvasPanelSlots();
	
	Gun->OnSpreadAmountChangedDelegate.BindUObject(this, &UCrosshair_Widget::HandleCrosshairScale);
	//Character->OnVelocityChangedDelegate.BindUObject(this, &UCrosshair_Widget::HandleCrosshairScale);
}

void UCrosshair_Widget::SetCanvasPanelSlots()
{
	CPS_Top = GetCanvasPanelSlot(SB_Top);
	CPS_Bottom = GetCanvasPanelSlot(SB_Bottom);
	CPS_Left = GetCanvasPanelSlot(SB_Left);
	CPS_Right = GetCanvasPanelSlot(SB_Right);
}

UCanvasPanelSlot* UCrosshair_Widget::GetCanvasPanelSlot(USizeBox* SizeBox)
{
	if (SizeBox)
	{
		return Cast<UCanvasPanelSlot>(SizeBox->Slot);
	}

	return nullptr;
}

void UCrosshair_Widget::HandleCrosshairScale(float Velocity)
{
	if (SB_Top && SB_Bottom && SB_Left && SB_Right) 
	{
		float FixedVelocity;
		//UE_LOG(LogTemp, Display, TEXT("velocity: %f"), Velocity);

		FixedVelocity = UKismetMathLibrary::NormalizeToRange(Velocity, 0.0f, MaxVelocity);
		//UE_LOG(LogTemp, Display, TEXT("from walking: %f"), FixedVelocity);

		FixedVelocity = FMath::Clamp(FixedVelocity, 0.0f, 1.0f);
		FixedVelocity *= MaxOffset;
		FVector2D Position;

		if (CPS_Top)
		{
			Position.Y = -FixedVelocity;
			Position.X = 0.0f;
			CPS_Top->SetPosition(Position);
		}
		else
		{
			CPS_Top = GetCanvasPanelSlot(SB_Top);
		}


		if (CPS_Bottom)
		{
			Position.Y = FixedVelocity;
			Position.X = 0.0f;
			CPS_Bottom->SetPosition(Position);
		}
		else
		{
			CPS_Bottom = GetCanvasPanelSlot(SB_Bottom);
		}


		if (CPS_Left)
		{
			Position.Y = 0.0f;
			Position.X = -FixedVelocity;
			CPS_Left->SetPosition(Position);
		}
		else
		{
			CPS_Left = GetCanvasPanelSlot(SB_Left);
		}


		if (CPS_Right)
		{
			Position.Y = 0.0f;
			Position.X = FixedVelocity;
			CPS_Right->SetPosition(Position);
		}
		else
		{
			CPS_Right = GetCanvasPanelSlot(SB_Right);
		}
	}
}
