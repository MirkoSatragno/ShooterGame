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
	/*Is the player requesting a WallRun action?
	* This variable is set to true for both requests*/
	bool bTriggeringWallRun;
	/*Is the player requesting a WallRunJump action?*/
	bool bTriggeringWallRunJump;
	
	/*Teleport action current availability*/
	bool bCanTeleport;
	/*WallJump action current availability
	*This is an additional parameter, different from the player state in space.
	*See GetCanWallJump()*/
	bool bCanWallJump;
	/*JetpackSprint action current availability*/
	bool bCanJetpackSprint;
	/*WallRun action current availability*/
	bool bCanWallRun;

	/*Stores Maximum ending time for WallRun, according to WallRunMaxDuration*/
	double WallRunMaxEndingTime;
	/*Stores Maximum ending time for WallRunJump, according to WallRunMaxJumpDelay*/
	double WallRunMaxJumpTime;
	/*The player can't WallRunJump twice in a row. Can he do it now?*/
	bool bWallRunJumpOnce;
	/** While MOVE_WallRunning idicates a physical state, that can be alternated with MOVE_Falling,
	* bWallRunFlowing indicates if the player is performing many chained WallRunning actions,
	* without touching the floor.*/
	bool bWallRunFlowing;
	/*Stored information about last grip point on an object*/
	FHitResult WallRunLastHitPoint;
	/*Vector storing WallRun instantaneous flowing direction.
	*It's constantly updated*/
	FVector WallRunFlowingDirection;
	

	/*Number of rays to be cast when checking for collisions all around*/
	uint8 WallRunWallDetectionRayNumber = 12;


	/**
	* Checks whether:
	* - the player is facing against a surface Normal direction
	* - the surface is close enough to the player
	*/
	bool IsWallInFrontOfPlayerValid() const;
	/**
	* Checks wether:
	* - the player is near a visible object
	* - the player is near a colliding object too (it can be the same)
	* - in case the player is already WallRunning, we check normal orientation of consecutive grip points on the wall
	*/
	bool IsWallNearPlayerValid(bool bSetGripPoint);
	/**
	* This is a custom version of the LineTraceSingleByChannel.
	* It casts RaysNumber rays around the player, all laying on the same XY-plane.
	* Hit informations returned are about the nearest hit, 
	* and it also checks that a visible object is present in that direction.
	*/
	bool CircleTraceSingleByChannel(struct FHitResult& OutHit, const FVector& Start, const FVector& End, uint8 RaysNumber) const;

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


	/*Maximum distance of a visible surface to be detected*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallRun")
		float WallRunMaxWallDetectionDistance = 80;
	/*Distance between the player and the wall when he is WallRunning*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallRun")
		float WallRunMaxWallSlidingDistance = 70;
	/*Maximum angle difference between two normals to consider them pointing in the "same" direction
	* This is used for both WallRun movement continuity and gripping on a new wall after a WallRunJump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "80"), Category = "WallRun")
		float WallRunMaxWallAngleVariation = 20;
	/*Maximum duration od WallRun on the same surface*/
	UPROPERTY(EditDefaultsOnly, meta = (ClapMin = "0", ClapMax = "1000"), Category = "WallRun")
		float WallRunMaxDuration = 3;
	/*Maximum time delay to perform a WallRunJump after stopping WallRun on a surface*/
	UPROPERTY(EditDefaultsOnly, meta = (ClapMin = "0", ClapMax = "1000"), Category = "WallRun")
		float WallRunMaxJumpDelay = 1;
	/*Movement speed while WallRunning*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "100000"), Category = "WallRun")
		float WallRunSpeed = 600;
	/*Acceleration while performing WallRunJump*/
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "10000000"), Category = "WallRun")
		float WallRunJumpLateralAcceleration = 80000;
	/*Vertical initial velocity when performing a WallRunJump, compared to regular jump vertical velocity*/
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
	/* WallRunStart or WallRunStop request state getter*/
	bool GetTriggeringWallRun() const;
	/* WallRunStart or WallRunStop request state setter*/
	void SetTriggeringWallRun(bool bTriggeringWallRun);
	/* WallRunJump request state getter*/
	bool GetTriggeringWallRunJump() const;
	/* WallRunJump request state setter*/
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

	/**
	* Following functions are also based on "bCan....." parameters,
	* but they can have additional conditions too.
	*/

	/*Teleport action current availability*/
	bool CanTeleport() const;
	/*WallJump action current availability*/
	bool CanWallJump() const;
	/*JetpackSprint action current availability*/
	bool CanJetpackSprint() const;
	/*WallRun action start current availability*/
	bool CanWallRun(bool bSetGripPoint);
	/*WallRun action stop current availability*/
	bool CanStopWallRun() const;
	/*WallRunJump action current availability*/
	bool CanWallRunJump() const;

	/*WallRunMaxEndingTime getter*/
	double GetWallRunMaxEndingTime() const;
	/*WallRunMaxEndingTime setter*/
	void SetWallRunMaxEndingTime(double WallRunMaxEndingTime);
	/*WallRunMaxJumpTime getter*/
	double GetWallRunMaxJumpTime() const;
	/*WallRunMaxJumpTime setter*/
	void SetWallRunMaxJumpTime(double WallRunMaxJumpTime);
	/*WallRunJumpOnce getter*/
	bool GetWallRunJumpOnce() const;
	/*WallRunJumpOnce setter*/
	void SetWallRunJumpOnce(bool bWallRunJumpOnce);
	/*WallRunFlowing getter*/
	bool GetWallRunFlowing() const;
	/*WallRunFlowing setter*/
	void SetWallRunFlowing(bool bWallRunFlowing);
	/*WallRunLastHitPoint getter*/
	const FHitResult* GetWallRunLastHitPoint() const;
	/*WallRunLastHitPoint setter*/
	void SetWallRunLastHitPoint(FHitResult WallRunLastHitPoint);
	/*WallRunFlowingDirection getter*/
	FVector GetWallRunFlowingDirection() const;
	/*WallRunFlowingDirection setter*/
	void SetWallRunFlowingDirection(FVector WallRunFlowingDirection);

	/*This works as an override in order to handle MOVE_WallRunning too*/
	void SetMovementMode(EMovementMode NewMovementMode);

	/*Is the Player performing WallRun movement?
	* Note that this is different from beeing in flowing state.*/
	UFUNCTION(BlueprintPure)
		bool IsWallRunning() const;
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
		FLAG_TriggeringJetpackSprint_WallRunJump = 0x40,	//this is used for both JetpackSprint and WallRunJump, that are mutually exclusive actions
		FLAG_TriggeringWallRun_WallRunJump = 0x80,	//this is used for both WallRun and WallRunJump, that are mutually exclusive actions
	};

	/*Stores bTriggeringTeleport value*/
	bool bSavedMove_TriggeringTeleport;
	/*Stores bTriggeringWallJump value*/
	bool bSavedMove_TriggeringWallJump;
	/*Stores bTriggeringJetpackSprint value*/
	bool bSavedMove_TriggeringJetpackSprint;
	/*Stores bTriggeringWallRun value*/
	bool bSavedMove_TriggeringWallRun;
	/*Stores bTriggeringWallRunJump*/
	bool bSavedMove_TriggeringWallRunJump;

	/*Stores b value*/
	bool bSavedMove_CanTeleport;
	/*Stores bCanWallJump value*/
	bool bSavedMove_CanWallJump;
	/*Stores bCanJetpackSprint value*/
	bool bSavedMove_CanJetpackSprint;
	/*Stores bCanWalllRun value*/
	bool bSavedMove_CanWallRun;

	/*Stores JetpackEnergy value*/
	double SavedMove_JetpackEnergy;
	/*Stores WallRunMaxEndingTime value*/
	double SavedMove_WallRunMaxEndingTime;
	/*Stores WallRunMaxJumpTime value*/
	double SavedMove_WallRunMaxJumpTime;
	/*Stores bWallRunJumpOnce value*/
	bool bSavedMove_WallRunJumpOnce;
	/*Stores bWallRunFlowing value*/
	bool bSavedMove_bWallRunFlowing;
	/*Stores WallRunLastHitPoint value*/
	FHitResult SavedMove_WallRunLastHitPoint;
	/*Stores WallRunFlowingDirection value*/
	FVector SavedMove_WallRunFlowingDirection;



	/* Clears SavedMove parameters */
	void Clear() override;
	/* Stores current action states from SavedMove into flags.
	* It's the opposite of UpdateFromCompressedFlags */
	uint8 GetCompressedFlags() const override;
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

