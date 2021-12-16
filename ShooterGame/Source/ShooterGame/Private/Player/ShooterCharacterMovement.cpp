// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetCanTeleport(true);
	SetCanWallJump(true);

	SetTriggeringTeleport(false);
	SetTriggeringWallJump(false);
	SetTriggeringJetpackSprint(false);


}


float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}

void UShooterCharacterMovement::PerformMovement(float DeltaTime) {

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (GetTriggeringTeleport()) {
		ShooterCharacter->Teleport();
		SetTriggeringTeleport(false);
	}

	if (GetTriggeringWallJump()) {
		ShooterCharacter->WallJump();
		SetTriggeringWallJump(false);
	}

	Super::PerformMovement(DeltaTime);

}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags) {

	Super::UpdateFromCompressedFlags(Flags);


	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return;

	if (ShooterCharacter->GetLocalRole() == ROLE_Authority) {
		int8 TeleportFlag = Flags & FSavedMove_Character_Upgraded::FLAG_TriggeringTeleport;

		if (TeleportFlag != 0)
			ShooterCharacter->Teleport();

		int8 WallJumpFlag = Flags & FSavedMove_Character_Upgraded::FLAG_TriggeringWallJump;

		if (WallJumpFlag != 0)
			ShooterCharacter->WallJump();
	}

}


class FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{

	checkSlow(CharacterOwner != NULL);
	checkSlow(CharacterOwner->GetLocalRole() < ROLE_Authority || (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy && GetNetMode() == NM_ListenServer));
	checkSlow(GetNetMode() == NM_Client || GetNetMode() == NM_ListenServer);

	if (ClientPredictionData == nullptr)
	{
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_Upgraded(*this);
	}

	return ClientPredictionData;
}


bool UShooterCharacterMovement::GetTriggeringTeleport() const
{
	return bTriggeringTeleport;
}

void UShooterCharacterMovement::SetTriggeringTeleport(bool bTriggeringTeleport)
{
	this->bTriggeringTeleport = bTriggeringTeleport;
}

bool UShooterCharacterMovement::GetTriggeringWallJump() const
{
	return bTriggeringWallJump;
}

void UShooterCharacterMovement::SetTriggeringWallJump(bool bTriggeringWallJump)
{
	this->bTriggeringWallJump = bTriggeringWallJump;
}

bool UShooterCharacterMovement::GetTriggeringJetpackSprint() const
{
	return bTriggeringJetpackSprint;
}

void UShooterCharacterMovement::SetTriggeringJetpackSprint(bool bTriggeringJetpackSprint)
{
	this->bTriggeringJetpackSprint = bTriggeringJetpackSprint;
}

bool UShooterCharacterMovement::GetCanTeleport() const
{
	return bCanTeleport;
}

void UShooterCharacterMovement::SetCanTeleport(bool bCanTeleport)
{
	this->bCanTeleport = bCanTeleport;
}

bool UShooterCharacterMovement::GetCanWallJump() const
{
	/** Two conditions are checked:
	* first one is the boolean bCanWallJump, that can be set to false when necessary;
	* second one is a check of the physical state of the player (is he falling, is he against a wall, etc.)*/
	
	if (!bCanWallJump)
		return false;

	return IsFalling() && IsWallInFrontOfPlayerValid();
}

void UShooterCharacterMovement::SetCanWallJump(bool bCanWallJump)
{
	this->bCanWallJump = bCanWallJump;
}

bool UShooterCharacterMovement::GetCanJetpackSprint() const 
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return false;
	
	if (!bCanWallJump)
		return false;

	return 0 < ShooterCharacter->GetJetpackEnergy();
}

void UShooterCharacterMovement::SetCanJetpackSrint(bool bCanJetpackSprint)
{
	this->bCanJetpackSprint = bCanJetpackSprint;
}


bool UShooterCharacterMovement::IsWallInFrontOfPlayerValid() const
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return false;
	
	/*For design's sake, the collision with a wall could be checked at lower height, like knees height*/
	float HeightCorrection = FMath::Clamp(RelativeCollisionHeight, (-1) * ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FVector StartLocation = ShooterCharacter->GetActorLocation() + HeightCorrection;
	FVector Direction = ShooterCharacter->Controller->GetControlRotation().Vector().GetSafeNormal2D();
	float PlayerCapsuleRadius = ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector EndLocation = StartLocation + Direction * (PlayerCapsuleRadius + MaxWallDistance);
	
	/*Raycast should not hit the character itself*/
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("WallTrace")), true, ShooterCharacter);

	/*Let's check for a collision in front of the player using a raycast*/
	FHitResult HitDetails = FHitResult(EForceInit::ForceInit);
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitDetails, StartLocation, EndLocation, ECC_Visibility, TraceParams);

	if (bIsHit) {
		/*Angle between player direction and inverted hit object normal
		* We want it to be lower than a custom threshold*/
		float ImpactAngle = FMath::Abs(Direction.Rotation().Yaw - (HitDetails.Normal.Rotation().Yaw - 180));
		/*If the angle is over 180, let's take its explementary value for a simpler MaxImpactAngle threshold comparison*/
		if (180 < ImpactAngle)
			ImpactAngle = 360 - ImpactAngle;

		if (ImpactAngle < MaxImpactAngle)
			return true;

	}
		

	return false;
}


////////////////////////////////
//FSavedMove_CharacterUpgraded 
////////////////////////////////

void FSavedMove_Character_Upgraded::Clear() {

	FSavedMove_Character::Clear();

	bSavedMove_TriggeringTeleport = false;
	bSavedMove_TriggeringWallJump = false;
}

uint8 FSavedMove_Character_Upgraded::GetCompressedFlags() const {

	uint8 Flags = FSavedMove_Character::GetCompressedFlags();

	if (bSavedMove_TriggeringTeleport)
		Flags |= FLAG_TriggeringTeleport;

	if (bSavedMove_TriggeringWallJump)
		Flags |= FLAG_TriggeringWallJump;

	return Flags;
}

bool FSavedMove_Character_Upgraded::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	UE_LOG(LogTemp, Warning, TEXT("CanCombineWith"));
	const FSavedMove_Character_Upgraded* NewMove_Upgraded = (FSavedMove_Character_Upgraded*)&NewMove;

	if (!NewMove_Upgraded || NewMove_Upgraded->bSavedMove_TriggeringTeleport)
		return false;

	if (NewMove_Upgraded->bSavedMove_TriggeringWallJump)
		return false;

	return FSavedMove_Character::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_Character_Upgraded::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov) {
		bSavedMove_TriggeringTeleport = CharMov->GetTriggeringTeleport();
		bSavedMove_TriggeringWallJump = CharMov->GetTriggeringWallJump();
	}
		
}

void FSavedMove_Character_Upgraded::PrepMoveFor(class ACharacter* Character)
{
	FSavedMove_Character::PrepMoveFor(Character);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov) {
		CharMov->SetTriggeringTeleport(bSavedMove_TriggeringTeleport);
		CharMov->SetTriggeringWallJump(bSavedMove_TriggeringWallJump);
	}

}


/////////////////////////////////////////////////////// 
//FNetworkPredictionData_Client_CharacterUpgraded 
/////////////////////////////////////////////////////// 

FSavedMovePtr FNetworkPredictionData_Client_Character_Upgraded::AllocateNewMove() 
{
	return FSavedMovePtr(new FSavedMove_Character_Upgraded());
}