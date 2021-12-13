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
	SetTriggeringTeleport(false);
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


bool UShooterCharacterMovement::GetCanTeleport() const
{
	return bCanTeleport;
}

void UShooterCharacterMovement::SetCanTeleport(bool CanTeleport)
{
	bCanTeleport = CanTeleport;
}



////////////////////////////////
//FSavedMove_CharacterUpgraded 
////////////////////////////////

void FSavedMove_Character_Upgraded::Clear() {

	FSavedMove_Character::Clear();

	bSavedMove_TriggeringTeleport = false;
}

uint8 FSavedMove_Character_Upgraded::GetCompressedFlags() const {

	uint8 Flags = FSavedMove_Character::GetCompressedFlags();

	if (bSavedMove_TriggeringTeleport)
		Flags |= FLAG_TriggeringTeleport;

	return Flags;
}

bool FSavedMove_Character_Upgraded::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	UE_LOG(LogTemp, Warning, TEXT("CanCombineWith"));
	const FSavedMove_Character_Upgraded* NewMove_Upgraded = (FSavedMove_Character_Upgraded*)&NewMove;

	if (!NewMove_Upgraded || NewMove_Upgraded->bSavedMove_TriggeringTeleport)
		return false;

	return FSavedMove_Character::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_Character_Upgraded::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov)
		bSavedMove_TriggeringTeleport = CharMov->GetTriggeringTeleport();
}

void FSavedMove_Character_Upgraded::PrepMoveFor(class ACharacter* Character)
{
	FSavedMove_Character::PrepMoveFor(Character);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov)
		CharMov->SetTriggeringTeleport(bSavedMove_TriggeringTeleport);
}


/////////////////////////////////////////////////////// 
//FNetworkPredictionData_Client_CharacterUpgraded 
/////////////////////////////////////////////////////// 

FSavedMovePtr FNetworkPredictionData_Client_Character_Upgraded::AllocateNewMove() 
{
	return FSavedMovePtr(new FSavedMove_Character_Upgraded());
}