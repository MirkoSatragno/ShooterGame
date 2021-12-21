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
	SetCanJetpackSprint(true);
	SetCanWallRun(true);

	SetTriggeringTeleport(false);
	SetTriggeringWallJump(false);
	SetTriggeringJetpackSprint(false);
	SetTriggeringWallRun(false);
	SetTriggeringWallRunJump(false);

	SetWallRunFlowing(false);
}


bool UShooterCharacterMovement::IsWallInFrontOfPlayerValid() const
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return false;

	/*For design's sake, the collision with a wall could be checked at lower height, like knees height*/
	float HeightCorrection = FMath::Clamp(WallJumpRelativeCollisionHeight, (-1) * ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FVector StartLocation = ShooterCharacter->GetActorLocation() + HeightCorrection;
	FVector Direction = ShooterCharacter->Controller->GetControlRotation().Vector().GetSafeNormal2D();
	float PlayerCapsuleRadius = ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector EndLocation = StartLocation + Direction * (PlayerCapsuleRadius + WallJumpMaxWallDistance);

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

		if (ImpactAngle < WallJumpMaxImpactAngle)
			return true;

	}


	return false;
}

bool UShooterCharacterMovement::IsWallNearPlayerValid()  
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return false;

	FVector StartLocation = ShooterCharacter->GetActorLocation();
	FVector Direction = ShooterCharacter->GetActorRotation().Vector();
	FVector ForwardRay = Direction  * (ShooterCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() + WallRunMaxWallDetectionDistance);
	

	FHitResult HitDetails = FHitResult(EForceInit::ForceInit);
	bool bIsHit = CircleTraceSingleByChannel(HitDetails, StartLocation, ForwardRay, ECC_Pawn, WallRunWallDetectionRayNumber);

	if (bIsHit) {
		WallRunLastHitPoint = HitDetails;
		return true;
	}


	return false;
}

bool UShooterCharacterMovement::CircleTraceSingleByChannel(struct FHitResult& OutHit, const FVector& Start, const FVector& ForwardRay, ECollisionChannel TraceChannel, uint8 RaysNumber) const
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return false;

	///*Collision check must not hit the character itself*/
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("WallTrace")), true, ShooterCharacter);

	float MinDist = FLT_MAX;
	for (int i = 0; i < RaysNumber; i++) {
		FVector End = Start + ForwardRay.RotateAngleAxis((360 / RaysNumber) * i, FVector::UpVector);
		FHitResult HitDetails = FHitResult(EForceInit::ForceInit);
		bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitDetails, Start, End, TraceChannel, TraceParams);
		if (bIsHit && HitDetails.Distance < MinDist) {
			OutHit = HitDetails;
			MinDist = OutHit.Distance;
		}


		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, true, 30.0f, 0, 3.0f);
	}

	return MinDist != FLT_MAX;
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

	ShooterCharacter->JetpackTick(DeltaTime);

	if (GetTriggeringWallRunJump()) {
		ShooterCharacter->WallRunJump();
		SetTriggeringWallRunJump(false);
		SetWallRunJumpOnce(false);
	}
	else {
		if (GetTriggeringWallRun()) {
			ShooterCharacter->WallRunChangeState();
			SetTriggeringWallRun(false);
		}
	}
	
	ShooterCharacter->WallRunTick(DeltaTime);

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

		bTriggeringJetpackSprint = Flags & FSavedMove_Character_Upgraded::FLAG_TriggeringJetpackSprint;

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


/**
* These action state getters and setters also enable/disable incompatible actions
**/
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

	/*The player can't WallJump while JetpackSprinting*/
	SetCanWallJump(!bTriggeringJetpackSprint);
}

bool UShooterCharacterMovement::GetTriggeringWallRun() const
{
	return bTriggeringWallRun && !GetTriggeringWallRunJump();
}

void UShooterCharacterMovement::SetTriggeringWallRun(bool bTriggeringWallRun)
{
	this->bTriggeringWallRun = bTriggeringWallRun;

	
	if (bTriggeringWallRun) {
		/*If the player is not WallRunning then he's gonna be, so he won't be able to JetpackSprint.
		*And viceversa if he is already WallRunning. */
		if (!IsWallRunning())
			SetCanJetpackSprint(false);
		if (IsWallRunning())
			SetCanJetpackSprint(true);
	}
	
}

bool UShooterCharacterMovement::GetTriggeringWallRunJump() const
{
	return bTriggeringWallRunJump;
}

void UShooterCharacterMovement::SetTriggeringWallRunJump(bool bTriggeringWallRunJump)
{
	this->bTriggeringWallRunJump = bTriggeringWallRunJump;
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
	return bCanWallJump;
}

void UShooterCharacterMovement::SetCanWallJump(bool bCanWallJump)
{
	this->bCanWallJump = bCanWallJump;
}

bool UShooterCharacterMovement::GetCanJetpackSprint() const
{
	return bCanJetpackSprint;
}

void UShooterCharacterMovement::SetCanJetpackSprint(bool bCanJetpackSprint)
{
	this->bCanJetpackSprint = bCanJetpackSprint;
}

bool UShooterCharacterMovement::GetCanWallRun()
{
	return bCanWallRun;
}

void UShooterCharacterMovement::SetCanWallRun(bool bCanWallRun)
{
	this->bCanWallRun = bCanWallRun;
}



bool UShooterCharacterMovement::CanTeleport() const
{
	return bCanTeleport;
}

bool UShooterCharacterMovement::CanWallJump() const
{
	/** Two conditions are checked:
	* first one is the boolean bCanWallJump, that can be set to false when necessary;
	* second one is a check of the physical state of the player (is he falling, is he against a wall, etc.)*/

	if (!bCanWallJump)
		return false;

	return IsFalling() && IsWallInFrontOfPlayerValid();
}

bool UShooterCharacterMovement::CanJetpackSprint() const
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(CharacterOwner);
	if (!ShooterCharacter)
		return false;

	if (!bCanJetpackSprint)
		return false;

	return 0 < ShooterCharacter->GetJetpackEnergy();
}

bool UShooterCharacterMovement::CanWallRun()
{
	if (!bCanWallRun)
		return false;

	return IsFalling() && IsWallNearPlayerValid();
}

bool UShooterCharacterMovement::CanStopWallRun() const
{
	return MovementMode == MOVE_WallRunning;
}

bool UShooterCharacterMovement::CanWallRunJump() const
{
	double TimeNow = GetWorld()->TimeSeconds;
	
	return IsFalling() && GetWallRunJumpOnce() && TimeNow < GetWallRunMaxJumpTime();
}



double UShooterCharacterMovement::GetWallRunMaxEndingTime() const
{
	return WallRunMaxEndingTime;
}

void UShooterCharacterMovement::SetWallRunMaxEndingTime(double WallRunMaxEndingTime)
{
	this->WallRunMaxEndingTime = WallRunMaxEndingTime;
}

double UShooterCharacterMovement::GetWallRunMaxJumpTime() const
{
	return WallRunMaxJumpTime;
}

void UShooterCharacterMovement::SetWallRunMaxJumpTime(double WallRunMaxJumpTime)
{
	this->WallRunMaxJumpTime = WallRunMaxJumpTime;
}

bool UShooterCharacterMovement::GetWallRunJumpOnce() const
{
	return bWallRunJumpOnce;
}

void UShooterCharacterMovement::SetWallRunJumpOnce(bool bWallRunJumpOnce) 
{
	this->bWallRunJumpOnce = bWallRunJumpOnce;
}

bool UShooterCharacterMovement::GetWallRunFlowing() const
{
	return bWallRunFlowing;
}

void UShooterCharacterMovement::SetWallRunFlowing(bool bWallRunFlowing)
{
	this->bWallRunFlowing = bWallRunFlowing;
}


void UShooterCharacterMovement::SetMovementMode(EMovementMode NewMovementMode) 
{
	if (NewMovementMode == MOVE_WallRunning) {
		/*This prevents that stored velocity is reapplied after WallRunning*/
		Velocity = FVector::ZeroVector;
	}

	Super::SetMovementMode(NewMovementMode);
}

const FHitResult* UShooterCharacterMovement::GetWallRunLastHitPoint() const
{
	return &WallRunLastHitPoint;
}

void UShooterCharacterMovement::SetWallRunLastHitPoint(FHitResult WallRunLastHitPoint) 
{
	this->WallRunLastHitPoint = WallRunLastHitPoint;
}

FVector UShooterCharacterMovement::GetWallRunFlowingDirection() const
{
	return WallRunFlowingDirection;
}

void UShooterCharacterMovement::SetWallRunFlowingDirection(FVector WallRunFlowingDirection)
{
	this->WallRunFlowingDirection = WallRunFlowingDirection;
}

bool UShooterCharacterMovement::IsWallRunning()
{
	return MovementMode == MOVE_WallRunning;
}


////////////////////////////////
//FSavedMove_CharacterUpgraded 
////////////////////////////////

void FSavedMove_Character_Upgraded::Clear() {

	FSavedMove_Character::Clear();

	bSavedMove_TriggeringTeleport = false;
	bSavedMove_TriggeringWallJump = false;
	bSavedMove_TriggeringJetpackSprint = false;
}

uint8 FSavedMove_Character_Upgraded::GetCompressedFlags() const {

	uint8 Flags = FSavedMove_Character::GetCompressedFlags();

	if (bSavedMove_TriggeringTeleport)
		Flags |= FLAG_TriggeringTeleport;

	if (bSavedMove_TriggeringWallJump)
		Flags |= FLAG_TriggeringWallJump;

	if (bSavedMove_TriggeringJetpackSprint)
		Flags |= FLAG_TriggeringJetpackSprint;

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

	if (NewMove_Upgraded->bSavedMove_TriggeringJetpackSprint)
		return false;

	return FSavedMove_Character::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_Character_Upgraded::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (!ShooterCharacter || !CharMov)
		return;

	bSavedMove_TriggeringTeleport = CharMov->GetTriggeringTeleport();
	bSavedMove_TriggeringWallJump = CharMov->GetTriggeringWallJump();
	bSavedMove_TriggeringJetpackSprint = CharMov->GetTriggeringJetpackSprint();
	bSavedMove_TriggeringWallRun = CharMov->GetTriggeringWallRun();
	bSavedMove_TriggeringWallRunJump = CharMov->GetTriggeringWallRunJump();

	bSavedMove_CanTeleport = CharMov->GetCanTeleport();
	bSavedMove_CanWallJump = CharMov->GetCanWallJump();
	bSavedMove_CanJetpackSprint = CharMov->GetCanJetpackSprint();
	bSavedMove_CanWallRun = CharMov->GetCanWallRun();

	SavedMove_JetpackEnergy = ShooterCharacter->GetJetpackEnergy();
	SavedMove_WallRunMaxEndingTime = CharMov->GetWallRunMaxEndingTime();
	SavedMove_WallRunMaxJumpTime = CharMov->GetWallRunMaxJumpTime();
	bSavedMove_WallRunJumpOnce = CharMov->GetWallRunJumpOnce();
}

void FSavedMove_Character_Upgraded::PrepMoveFor(class ACharacter* Character)
{
	FSavedMove_Character::PrepMoveFor(Character);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(Character);
	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (!ShooterCharacter || !CharMov)
		return;

	CharMov->SetTriggeringTeleport(bSavedMove_TriggeringTeleport);
	CharMov->SetTriggeringWallJump(bSavedMove_TriggeringWallJump);
	CharMov->SetTriggeringJetpackSprint(bSavedMove_TriggeringJetpackSprint);
	CharMov->SetTriggeringWallRun(bSavedMove_TriggeringWallRun);

	CharMov->SetCanTeleport(bSavedMove_CanTeleport);
	CharMov->SetCanWallJump(bSavedMove_CanWallJump);
	CharMov->SetCanJetpackSprint(bSavedMove_CanJetpackSprint);

	ShooterCharacter->SetJetpackEnergy(SavedMove_JetpackEnergy);

}


/////////////////////////////////////////////////////// 
//FNetworkPredictionData_Client_CharacterUpgraded 
/////////////////////////////////////////////////////// 

FSavedMovePtr FNetworkPredictionData_Client_Character_Upgraded::AllocateNewMove() 
{
	return FSavedMovePtr(new FSavedMove_Character_Upgraded());
}