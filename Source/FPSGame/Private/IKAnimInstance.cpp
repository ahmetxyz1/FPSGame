// Fill out your copyright notice in the Description page of Project Settings.
#include "IKAnimInstance.h"
#include "FPSGame/FPSGameCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveVector.h"

UIKAnimInstance::UIKAnimInstance()
{
	AimAlpha = 0.0f;
	bInterpAiming = false;
	bIsAiming = false;
	bInterpRelativeHand = false;
	ReloadAlpha = 1.0f;
}

void UIKAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<AFPSGameCharacter>(TryGetPawnOwner());

	if (Character)
	{
		FTimerHandle TSetSightTransform;
		FTimerHandle TSetRelativeHandTransform;
		//timer for the function to start then this gets destroyed i tink 
		GetWorld()->GetTimerManager().SetTimer(TSetSightTransform, this, &UIKAnimInstance::SetSightTransform, 0.25f, false);
		GetWorld()->GetTimerManager().SetTimer(TSetRelativeHandTransform, this, &UIKAnimInstance::SetRelativeHandTransform, 0.25f, false);

		OldRotation = Character->GetControlRotation();
	}
}

void UIKAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character) { return; }

	if (bInterpAiming)
	{
		InterpAiming(DeltaSeconds);
	}

	if (bInterpRelativeHand)
	{
		InterpRelativeHand(DeltaSeconds);
	}

	if (Character->IsLocallyControlled())
	{
		RotateWithRotation(DeltaSeconds);
		MoveVectorCurve(DeltaSeconds);

		if (!RecoilTransform.Equals(FTransform()) || !FinalRecoilTransform.Equals(FTransform()))
		{
			InterpRecoil(DeltaSeconds);
			InterpFinalRecoil(DeltaSeconds);
		}
	}

	SetLeftHandIK();
}

void UIKAnimInstance::SetSightTransform()
{
	FTransform CamTransform = Character->GetFirstPersonCameraComponent()->GetComponentTransform();
	FTransform MeshTransform = Character->GetMesh1P()->GetComponentTransform();

	SightTransform = UKismetMathLibrary::MakeRelativeTransform(CamTransform, MeshTransform);
	SightTransform.SetLocation(SightTransform.GetLocation() + SightTransform.GetRotation().Vector() * 25.0f);
}

void UIKAnimInstance::SetRelativeHandTransform()
{
	if (Character->GetCurrentOptic())
	{
		FTransform OpticSocketTransform = Character->GetCurrentOptic()->GetSocketTransform(FName("S_Aim"));
		FTransform MeshTransform = Character->GetMesh1P()->GetSocketTransform(FName("hand_r"));

		RelativeHandTransform = UKismetMathLibrary::MakeRelativeTransform(OpticSocketTransform, MeshTransform);
	}
}

void UIKAnimInstance::SetFinalHandTransform()
{
	if (Character->GetCurrentOptic())
	{
		FTransform OpticSocketTransform = Character->GetCurrentOptic()->GetSocketTransform(FName("S_Aim"));
		FTransform MeshTransform = Character->GetMesh1P()->GetSocketTransform(FName("hand_r"));

		FinalHandTransform = UKismetMathLibrary::MakeRelativeTransform(OpticSocketTransform, MeshTransform);
	}
}

void UIKAnimInstance::SetLeftHandIK()
{
	FTransform GunSocketTransform = Character->GetFPGun()->GetSocketTransform(FName("S_LeftHand"));
	FTransform MeshSocketTransform = Character->GetMesh1P()->GetSocketTransform(FName("hand_r"));

	LeftHandTransform = UKismetMathLibrary::MakeRelativeTransform(GunSocketTransform, MeshSocketTransform);
}

void UIKAnimInstance::InterpAiming(float DeltaSeconds)
{
	AimAlpha = UKismetMathLibrary::FInterpTo(AimAlpha, static_cast<float>(bIsAiming), DeltaSeconds, 20.0f);

	if (AimAlpha >= 1.0f || AimAlpha <= 0.0f)
	{
		bInterpAiming = false;
	}
}

void UIKAnimInstance::InterpRelativeHand(float DeltaSeconds)
{
	RelativeHandTransform = UKismetMathLibrary::TInterpTo(RelativeHandTransform, FinalHandTransform, DeltaSeconds, 20.0f);

	if (RelativeHandTransform.Equals(FinalHandTransform))
	{
		bInterpRelativeHand = false;
	}
}

void UIKAnimInstance::MoveVectorCurve(float DeltaSeconds)
{
	if (VectorCurve)
	{
		FVector VelocityVec = Character->GetMovementComponent()->Velocity;
		VelocityVec.Z = 0.0f;
		float Velocity = VelocityVec.Size();
		float MaxSpeed = Character->GetMovementComponent()->GetMaxSpeed();
		Velocity = UKismetMathLibrary::NormalizeToRange(Velocity, (MaxSpeed / 1.0f * -1.0f), MaxSpeed);
		FVector NewVec = VectorCurve->GetVectorValue(Character->GetGameTimeSinceCreation());
		SwayLocation = UKismetMathLibrary::VInterpTo(SwayLocation, NewVec, DeltaSeconds, 6.0f);
		SwayLocation *= Velocity;
	}
}

void UIKAnimInstance::RotateWithRotation(float DeltaSeconds)
{
	FRotator CurrentRotation = Character->GetControlRotation();
	UnmodifiedTurnRotator = UKismetMathLibrary::RInterpTo(UnmodifiedTurnRotator, CurrentRotation - OldRotation, DeltaSeconds, 4.0f);
	FRotator TurnRotation = UnmodifiedTurnRotator;
	TurnRotation.Roll = TurnRotation.Pitch;
	TurnRotation.Pitch = 0.0f;

	TurnRotation.Yaw = FMath::Clamp(TurnRotation.Yaw, -7.0f, 7.0f) * -1.0f;
	TurnRotation.Roll = FMath::Clamp(TurnRotation.Roll, -3.0f, 3.0f);

	FVector TurnLocation;
	TurnLocation.X = TurnRotation.Yaw / 4;
	TurnLocation.Z = TurnRotation.Roll / 1.5;

	TurningSwayTransform.SetLocation(TurnLocation);
	TurningSwayTransform.SetRotation(TurnRotation.Quaternion());

	OldRotation = CurrentRotation;
}

void UIKAnimInstance::SetAiming(bool IsAiming)
{
	if (bIsAiming != IsAiming)
	{
		bIsAiming = IsAiming;
		bInterpAiming = true;
	}
}

void UIKAnimInstance::CycledOptic()
{
	SetFinalHandTransform();
	bInterpRelativeHand = true;
}

void UIKAnimInstance::Reload()
{
	if (ReloadAlpha == 1.0f)
	{
		ReloadAlpha = 0.0f;
	}
	else if (ReloadAlpha == 0.0f)
	{
		ReloadAlpha = 1.0f;
	}
}

void UIKAnimInstance::StopReload()
{
	ReloadAlpha = 1.0f;
}

void UIKAnimInstance::InterpFinalRecoil(float DeltaSeconds)
{ //interp to 0
	FinalRecoilTransform = UKismetMathLibrary::TInterpTo(FinalRecoilTransform, FTransform(), DeltaSeconds, 15.0f);
}

void UIKAnimInstance::InterpRecoil(float DeltaSeconds)
{ // interp to finalrecoiltransform
	RecoilTransform = UKismetMathLibrary::TInterpTo(RecoilTransform, FinalRecoilTransform, DeltaSeconds, 15.0f);
}

void UIKAnimInstance::Fire()
{
	FVector RecoilLoc = FinalRecoilTransform.GetLocation();
	//                                        x left and right       y pull back rate                 z up and down
	RecoilLoc += FVector(FMath::RandRange(0.08f, 0.08f), FMath::RandRange(-1.0f, 3.5f), FMath::RandRange(-0.1f, 1.0f));

	FRotator RecoilRot = FinalRecoilTransform.GetRotation().Rotator();
	//pictch is roll. roll is b
	RecoilRot += FRotator(FMath::RandRange(-4.0f, 4.0f), FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-3.5f, -1.5f));

	FinalRecoilTransform.SetRotation(RecoilRot.Quaternion());
	FinalRecoilTransform.SetLocation(RecoilLoc);
}