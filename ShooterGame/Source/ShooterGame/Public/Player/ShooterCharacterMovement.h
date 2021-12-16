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

	/*Has the player requested a Teleport action?*/
	bool bTriggeringTeleport;
	/*Has the player requested a WallJump action?*/
	bool bTriggeringWallJump;
	/*Is the player requesting a JetpackSprint action?*/
	bool bTriggeringJetpackSprint;
	
	/*Teleport action current availability*/
	bool bCanTeleport;
	/*WallJump action current availability
	*This is an additional parameter, different from the player state in space.
	*See GetCanWallJump()*/
	bool bCanWallJump;
	/*JetpackSprint action current availability*/
	bool bCanJetpackSprint;

	


public:

	/*Distance traveled with teleport action, measured in cm*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "Teleport")
		float TeleportDistance = 1000;
	

	/*Maximum distance of the wall from the player to perform a wall jump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallJump")
		float MaxWallDistance = 10;
	/*Maximum character rotation towards the wall to perform a wall jump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "90"), Category = "WallJump")
		float MaxImpactAngle = 10;
	/*Height of the collision point to be checked, relative to Player's center*/
	UPROPERTY(EditDefaultsOnly, Category = "WallJump")
		float RelativeCollisionHeight = 0;
	/*Upward force applied to the player performing the wall jump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100"), Category = "WallJump")
		float JumpVelocityModifier = 1;
	/*Force that pushes the actor away from the wall*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "10000000"), Category = "WallJump")
		float ResponseImpulseIntensity = 20000;


	/*Force that pushes the actor upward*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "JetpackSprint")
		float JetpackUpwardAcceleration = 3000;



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
	/* WallJump request state getter*/
	bool GetTriggeringWallJump() const;
	/* WallJump request state setter*/
	void SetTriggeringWallJump(bool bTriggeringWallJump);
	/* JetpackSprint request state getter*/
	bool GetTriggeringJetpackSprint() const;
	/* JetpackSprint request state setter*/
	void SetTriggeringJetpackSprint(bool bTriggeringJetpackSprint);

	/*Teleport action current availability getter*/
	bool GetCanTeleport() const;
	/*Teleport action current availability setter*/
	void SetCanTeleport(bool bCanTeleport);
	/*WallJump action current availability getter.
	*It also checks if player's state alow this movement*/
	bool GetCanWallJump() const;
	/*WallJump action current availability setter*/
	void SetCanWallJump(bool bCanWallJump);

	bool GetCanJetpackSprint() const;

	void SetCanJetpackSrint(bool bCanJetpackSprint);

	/**
	* Checks wether:
	* - the player is facing against a surface Normal direction
	* - the surface is close enough to the player
	*/
	bool IsWallInFrontOfPlayerValid() const;

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
		FLAG_TriggeringWallJump = 0x20,
		FLAG_Custom_2 = 0x40,
		FLAG_Custom_3 = 0x80,
	};

	/*Stores bTriggeringTeleport value*/
	bool bSavedMove_TriggeringTeleport;
	/*Stores bTriggeringWallJump value*/
	bool bSavedMove_TriggeringWallJump;

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

