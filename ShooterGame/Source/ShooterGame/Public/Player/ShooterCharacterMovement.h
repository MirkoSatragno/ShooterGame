// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

private:

	/*Has actor requested a Teleport action?*/
	bool bTriggeringTeleport;
	
	/*Teleport action current availability*/
	bool bCanTeleport;


public:

	virtual float GetMaxSpeed() const override;

	/**Locally performs the movement.*/
	void PerformMovement(float DeltaTime) override;
	/**Updates local state from flags stored in a SavedMove.*/
	void UpdateFromCompressedFlags(uint8 Flags) override;
	/** Allocates my custom FNetworkPredictionData_Client_Character_Upgraded,
	* Instead of the default FNetworkPredictionData_Client_Character*/
	class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/* Teleport request state getter*/
	bool GetTriggeringTeleport() const;
	/* Teleport request state setter*/
	void SetTriggeringTeleport(bool bTriggeringTeleport);

	/*Teleport action current availability getter*/
	bool GetCanTeleport() const;
	/*Teleport action current availability setter*/
	void SetCanTeleport(bool CanTeleport);

};


class FSavedMove_Character_Upgraded : public FSavedMove_Character
{
public:

	enum CompressedFlagsExtended
	{
		FLAG_JumpPressed = 0x01,	// Jump pressed 
		FLAG_WantsToCrouch = 0x02,	// Wants to crouch 
		FLAG_Reserved_1 = 0x04,	// Reserved for future use 
		FLAG_Reserved_2 = 0x08,	// Reserved for future use 
		// Remaining bit masks are available for custom flags. 
		FLAG_TriggeringTeleport = 0x10,
		FLAG_Custom_1 = 0x20,
		FLAG_Custom_2 = 0x40,
		FLAG_Custom_3 = 0x80,
	};

	/*Stores bTriggeringTeleport value*/
	bool bSavedMove_TriggeringTeleport;

	/* Clears SavedMove parameters */
	void Clear() override;
	/* Stores current action states from SavedMove into flags.
	* It's the opposite of UpdateFromCompressedFlags */
	uint8 GetCompressedFlags() const override;
	/* Checks if two moves are actually the same one that is still active */
	bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	/* Stores current action states from local state into SavedMove.
	* Stored data will be used to replay actions if needed */
	void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	/* Updates local action states from data stored in SavedMove */
	void PrepMoveFor(class ACharacter* Character) override;

};



class FNetworkPredictionData_Client_Character_Upgraded : public FNetworkPredictionData_Client_Character
{
public:

	FNetworkPredictionData_Client_Character_Upgraded(const UCharacterMovementComponent& ClientMovement) : FNetworkPredictionData_Client_Character(ClientMovement) {};

	/** Allocates my custom FSavedMove_Character_Upgraded,
	* Instead of the default FSavedMove_Character*/
	FSavedMovePtr AllocateNewMove() override;
};

