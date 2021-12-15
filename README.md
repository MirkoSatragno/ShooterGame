# ShooterGame

Added features:
- Teleport
- WallJump

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