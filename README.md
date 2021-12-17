# ShooterGame

Added features:
- Teleport
- WallJump
- JetpackSprint

## Teleport action description
- The player can teleport 10 meters in forward direction.
- Distance parameter can be modified in Blueprint editor.
- Forward direction follows player's view direction, with z-component too.
- Teleport can be performed through walls and obstacles, as long as the destination is obstacle free.
- Teleport can be performed through floors and ceilings too.
- Teleport action is not constrained by energy resources or timers.

## WallJump action description
- The player can WallJump when he is facing a visible surface.
- WallJump can't be performed on invisible colliders, such as invisible area limits.
- It is possible to perform multiple WallJumps in a row, if the level architectures allow it.
- The player needs to align to the wall Normal direction in order to perform a WallJump.
- The power of the jump, the wall distance and rotation limits, and other parameters can be customized in blueprint editor.

## JetpackSprint action description
- JetpackSprint action applies a constant force to the player as long as he presses J button.
- When the JetpackSprint is working, energy is consumed at a customizable rate.
- When the energy is over, the JetpackSprint action stops working.
- When the JetpackSprint ability is not being used, the energy regenerates at a customizable rate.
- The player can't WallJump when he is JetpackSprinting.
- Falling after using JetpackSprint ability doesn't count as "still using the ability", thus energy does regenerate while falling, and WallJump in available again.
- After the energy is over, the player needs to release J button and then press it again, if he wills to JetpackSprinting again.
- A bar on the left-hand side shows the energy available.
