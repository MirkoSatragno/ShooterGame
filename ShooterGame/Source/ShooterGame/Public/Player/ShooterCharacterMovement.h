// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "ShooterCharacterMovement.generated.h"

#define MOVE_WallRunning MOVE_Custom

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
	bool bTriggeringWallRun;
	bool bTriggeringWallRunJump;
	
	/*Teleport action current availability*/
	bool bCanTeleport;
	/*WallJump action current availability
	*This is an additional parameter, different from the player state in space.
	*See GetCanWallJump()*/
	bool bCanWallJump;
	/*JetpackSprint action current availability*/
	bool bCanJetpackSprint;
	bool bCanWallRun;

	/*Stores Maximum ending time for WallRun, according to WallRunMaxDuration*/
	double WallRunMaxEndingTime;
	double WallRunMaxJumpTime;
	bool bWallRunJumpOnce;
	/** While MOVE_WallRunning idicates a physical state, that can be alternated with MOVE_Falling,
	* bWallRunFlowing indicates if the player is performing many chained WallRunning actions,
	* without touching the floor.*/
	bool bWallRunFlowing;
	FHitResult WallRunLastHitPoint;
	FVector WallRunFlowingDirection;
	


	float WallRunWallCheckCollisionCapsuleHalfHeight = 1;
	uint8 WallRunWallDetectionRayNumber = 12;


	/**
	* Checks whether:
	* - the player is facing against a surface Normal direction
	* - the surface is close enough to the player
	*/
	bool IsWallInFrontOfPlayerValid() const;
	bool IsWallNearPlayerValid();
	bool CircleTraceSingleByChannel(struct FHitResult& OutHit, const FVector& Start, const FVector& End, ECollisionChannel TraceChannel, uint8 RaysNumber) const;

public:

	/*Distance traveled with teleport action, measured in cm*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "Teleport")
		float TeleportDistance = 1000;
	

	/*Maximum distance of the wall from the player to perform a wall jump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallJump")
		float WallJumpMaxWallDistance = 40;
	/*Maximum character rotation towards the wall to perform a wall jump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "90"), Category = "WallJump")
		float WallJumpMaxImpactAngle = 10;
	/*Height of the collision point to be checked, relative to Player's center*/
	UPROPERTY(EditDefaultsOnly, Category = "WallJump")
		float WallJumpRelativeCollisionHeight = 0;
	/*Upward force applied to the player performing the wall jump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100"), Category = "WallJump")
		float WallJumpVelocityModifier = 1;
	/*Force that pushes the actor away from the wall*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "10000000"), Category = "WallJump")
		float WallJumpResponseImpulseIntensity = 20000;


	/*Force that pushes the actor upward*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "JetpackSprint")
		float JetpackUpwardAcceleration = 3000;
	/*Jetpack Energy Per Second consumption*/
	UPROPERTY(EditDefaultsOnly, meta = (ClapMin = "0", ClapMax = "1000"), Category = "JetpackSprint")
		int32 JetpackEPS = 10;
	/*Jetpack Energy Per Second recharge*/
	UPROPERTY(EditDefaultsOnly, meta = (ClapMin = "0", ClapMax = "1000"), Category = "JetpackSprint")
		int32 JetpackRechargeEPS = 10;


	
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallRun")
		float WallRunMaxWallDetectionDistance = 50;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallRun")
		float WallRunMaxWallSlidingDistance = 40;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "80"), Category = "WallRun")
		float WallRunMaxWallAngleVariation = 5;
	/*Maximum duration od WallRun on the same surface*/
	UPROPERTY(EditDefaultsOnly, meta = (ClapMin = "0", ClapMax = "1000"), Category = "WallRun")
		float WallRunMaxDuration = 3;
	UPROPERTY(EditDefaultsOnly, meta = (ClapMin = "0", ClapMax = "1000"), Category = "WallRun")
		float WallRunMaxJumpDelay = 1;
	/*Movement speed while WallRunning*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallRun")
		float WallRunSpeed = 300;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "10000000"), Category = "WallRun")
		float WallRunJumpLateralAcceleration = 30000;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100"), Category = "WallRun")
		float WallRunJumpVerticalVelocityModifier = 2;


	virtual float GetMaxSpeed() const override;

	/**Locally performs the movement.*/
	void PerformMovement(float DeltaTime) override;
	/**Updates local state from flags stored in a SavedMove.*/
	void UpdateFromCompressedFlags(uint8 Flags) override;
	/** Allocates my custom FNetworkPredictionData_Client_Character_Upgraded,
	* Instead of the default FNetworkPredictionData_Client_Character*/
	class FNetworkPredictionData_Client* GetPredictionData_Client() const override;


	/**
	* These action state getters and setters also enable/disable incompatible actions
	**/
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

	bool GetTriggeringWallRun() const;
	void SetTriggeringWallRun(bool bTriggeringWallRun);
	bool GetTriggeringWallRunJump() const;
	void SetTriggeringWallRunJump(bool bTriggeringWallRunJump);


	/*Teleport action current availability getter*/
	bool GetCanTeleport() const;
	/*WallJump action current availability getter*/
	bool GetCanWallJump() const;
	/*JetpackSprint action current availability getter*/
	bool GetCanJetpackSprint() const;
	/*WallRun action current availbility getter*/
	bool GetCanWallRun();
	/*Teleport action current availability setter*/
	void SetCanTeleport(bool bCanTeleport);
	/*WallJump action current availability setter*/
	void SetCanWallJump(bool bCanWallJump);
	/*JetpackSprint action current availability setter*/
	void SetCanJetpackSprint(bool bCanJetpackSprint);
	/*WallRun action current availability setter*/
	void SetCanWallRun(bool bCanWallRun);


	bool CanTeleport() const;
	bool CanWallJump() const;
	bool CanJetpackSprint() const;
	bool CanWallRun();
	bool CanStopWallRun() const;
	bool CanWallRunJump() const;

	double GetWallRunMaxEndingTime() const;
	void SetWallRunMaxEndingTime(double WallRunMaxEndingTime);
	double GetWallRunMaxJumpTime() const;
	void SetWallRunMaxJumpTime(double WallRunMaxJumpTime);
	bool GetWallRunJumpOnce() const;
	void SetWallRunJumpOnce(bool bWallRunJumpOnce);
	bool GetWallRunFlowing() const;
	void SetWallRunFlowing(bool bWallRunFlowing);
	const FHitResult* GetWallRunLastHitPoint() const;
	void SetWallRunLastHitPoint(FHitResult WallRunLastHitPoint);
	FVector GetWallRunFlowingDirection() const;
	void SetWallRunFlowingDirection(FVector WallRunFlowingDirection);

	void SetMovementMode(EMovementMode NewMovementMode);

	/*Is the Player performing WallRun movement?*/
	UFUNCTION(BlueprintPure)
		bool IsWallRunning();
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
		FLAG_TriggeringJetpackSprint = 0x40,
		FLAG_Custom_3 = 0x80,
	};

	/*Stores bTriggeringTeleport value*/
	bool bSavedMove_TriggeringTeleport;
	/*Stores bTriggeringWallJump value*/
	bool bSavedMove_TriggeringWallJump;
	/*Stores bTriggeringJetpackSprint value*/
	bool bSavedMove_TriggeringJetpackSprint;
	bool bSavedMove_TriggeringWallRun;
	bool bSavedMove_TriggeringWallRunJump;

	/*Stores b value*/
	bool bSavedMove_CanTeleport;
	/*Stores bCanWallJump value*/
	bool bSavedMove_CanWallJump;
	/*Stores bCanJetpackSprint value*/
	bool bSavedMove_CanJetpackSprint;
	bool bSavedMove_CanWallRun;

	/*Stores JetpackEnergy value*/
	double SavedMove_JetpackEnergy;
	double SavedMove_WallRunMaxEndingTime;
	double SavedMove_WallRunMaxJumpTime;
	bool bSavedMove_WallRunJumpOnce;



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

