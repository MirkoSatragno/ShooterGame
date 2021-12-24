# ShooterGame

Added features:
- Teleport
- WallJump
- JetpackSprint
- WallRun

## Teleport action description
- The ability is triggered pressing T-Key in any ciscumstance
- The player can teleport 10 meters in forward direction.
- Distance parameter can be modified in Blueprint editor.
- Forward direction follows player's view direction, with z-component too.
- Teleport can be performed through walls and obstacles, as long as the destination is obstacle free.
- Teleport can be performed through floors and ceilings too.
- Teleport action is not constrained by energy resources or timers.

## WallJump action description
- WallJump can be perfomed pushing the space bar when all the action requirements are met.
- The player can WallJump when he is facing a visible surface within a customizable distance.
- WallJump can't be performed on invisible colliders, such as invisible area limits.
- It is possible to perform multiple WallJumps in a row, if the level architectures allow it.
- The player needs to align to the wall Normal direction in order to perform a WallJump.
- The player can't WallJump if the target wall is on side side or his back. However, for the sake of experimentation, such implementation can be found in WallRun action.
- The power of the jump, the wall distance and angle limits, and other parameters can be customized in Blueprint editor.

## JetpackSprint action description
- JetpackSprint can be triggered pushing J-Key
- JetpackSprint action applies a constant force to the player as long as he presses J-Key.
- When the JetpackSprint is working, energy is consumed at a customizable rate.
- When the energy is over, the JetpackSprint action stops working.
- When the JetpackSprint ability is not being used, the energy regenerates at a customizable rate.
- The player can't WallJump when he is JetpackSprinting.
- Falling after using JetpackSprint ability doesn't count as "still using the ability", thus energy does regenerate while falling, and WallJump in available again.
- After the energy is over, the player needs to release J button and then press it again, if he wills to JetpackSprinting again.
- A bar on the left-hand side shows the energy available.

## WallRun action description
- The player can WallRun and WallRunJump pressing X-Key.
- The player needs to be near to a visible surface in order to WallRun.
- The player performs WallRunning on colliding objects, even though visible meshes are required.
- The player can't WallRun on invisible area limits.
- WallRun action is performed as long as the player keeps X button pushed.
- There is a maximum time limit to slide on a single surface, that can be cutomized by Blueprint editor.
- When WallRun action is interrupted, the actor starts falling with a residual movement velocity.
- After the player stops WallRunning, for any reason, he has a customizable time limit to perform a WallRunJump.
- The player can perform a WallRunJump in order to grip on another wall and continuing his WallRunning.
- WallRunJump is performed towards player view direction.
- Jumping on another wall whit WallRunJump requires the wall to have a different Normal direction from the previous wall surface grip pont.
- Many speed, acceleration, angle limits, time limits, etc., parameters can be customized in Blueprint editor.
- When the player is attached to the wall, the first person mesh animation is changed to JumpingLoop.
- The third person mesh animation has not been changed.
