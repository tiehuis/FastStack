///
// `fs.c`
//
// Header file for the FastStack engine.
//
// The engine is mostly opaque to an outside user. A number of functions are
// provided which provide some convenience when performing certain tasks.
///

#ifndef FS_H
#define FS_H

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "fsConfig.h"
#include "fsControl.h"
#include "fsTypes.h"
#include "fsLog.h"

/// Name of configuration file.
#define FS_CONFIG_FILENAME "fs.ini"

/// Number of types of pieces.
#define FS_NPT 7

/// Number of rotation systems.
#define FS_NRS 5

/// Number of rotation states.
#define FS_NPR 4

/// Number of blocks in a piece.
#define FS_NBP 4

/// Piece types
enum PieceType {
    FS_I,
    FS_J,
    FS_L,
    FS_O,
    FS_S,
    FS_T,
    FS_Z,
    FS_NONE
};

/// Randomizer type
enum RandomizerType {
    FSRAND_UNDEFINED,
    FSRAND_SIMPLE,
    FSRAND_NOSZO_BAG7,
    FSRAND_TGM1,
    FSRAND_TGM2
};

/// Rotation System type
enum RotationSystemType {
    FSROT_SIMPLE,
    FSROT_SRS,
    FSROT_ARIKA_SRS,
    FSROT_TGM12,
    FSROT_DTET
};

/// Rotation amount
enum RotationAmount {
    FSROT_CLOCKWISE = 1,
    FSROT_ANTICLOCKWISE = -1,
    FSROT_HALFTURN = 2
};

///
// Locking System type.
///
enum LockStyle {
    /// Lock delay is reset only on entry of a new piece.
    FSLOCK_ENTRY,

    /// Lock delay is reset on any downwards movement.
    FSLOCK_STEP,

    /// Lock delay is reset on any **successful** movement.
    FSLOCK_MOVE
};

///
// All possible game states.
///
enum GameState {
    /// Occurs whilst 'READY' is displayed
    FSS_READY,

    /// Occurs whilst 'GO' is displayed
    FSS_GO,

    /// Occurs when a piece has nothing beneath it.
    FSS_FALLING,

    /// Occurs when a piece has hit the top of the stack/floor.
    FSS_LANDED,

    /// Occurs when waiting for a new piece to spawn (entry delay)
    FSS_ARE,

    /// Occurs when a new piece needs to be spawned. This occurs instantly.
    FSS_NEW_PIECE,

    /// (unused) Occurs when a line clear is occurring.
    FSS_LINES,

    /// Occurs when a user-specified quit action occurred.
    FSS_QUIT,

    /// Occurs when the user lost (topped out).
    FSS_GAMEOVER,

    /// Unknown state
    FSS_UNKNOWN
};

///
// A wallkick table consists of a number 'tests' which are tested in order
// until success or every test has been tried.
///
typedef FSInt3 WallkickTable[FS_NPR][FS_MAX_KICK_LEN];

///
// Specifies a single rotation system.
//
// A rotation system is comprised of three main parts:
//
//  - Entry Offsets
//      Specifies x, y offsets of a piece when it initially spawns.
//
//  - Entry Theta
//      Specifies the rotation state of a piece when it initially spawns.
//
//  - Kick Tables and Kick Indexes
//      Specifies individual wallkick tables for a given piece. Tables can
//      be shared amongst types by reusing the index.
///
typedef struct FSRotationSystem {
    /* Initial x, y offsets. */
    FSInt entryOffset[FS_NPT];

    /* Initial theta offets. */
    FSInt entryTheta[FS_NPT];

    /* Indexes into 'kickTables'. */
    FSInt kicksL[FS_NPT];
    FSInt kicksR[FS_NPT];
    FSInt kicksH[FS_NPT];

    /* A sequence of wallkick tests. */
    WallkickTable kickTables[FS_MAX_NO_OF_WALLKICK_TABLES];
} FSRotationSystem;

///
// Rotation Systems are defined statically. We only store an index to the
// currently used table in 'FSGame'.
///
extern const FSRotationSystem *rotationSystems[FS_NRS];

///
// An empty wallkick table.
///
extern const WallkickTable emptyWallkickTable;

///
// A single FastStack game instance.
//
// Stores all internal variables and options pertaining to a field.
// Values can be broken down into one of three classes.
//
//  - Internal Status (@I)
//      Only used internally and never required to be read by a platform.
//
//  - External Status (@E)
//      Calculated internally by the engine, but expected to be read by a
//      user.
//
//  - Fixed Option (@O)
//      Can be set by the user. Typically unsafe to change during execution.
//
//  We document which of the following variables belongs to which class. These
//  are only guidelines and there may be cases where we need to break the
//  following visibility rules.
//
//  Note: ANy 'Constraints' should always be true at any point in time.
///
typedef struct FSGame {
    /// @E: Current field state.
    FSBlock b[FS_MAX_HEIGHT][FS_MAX_WIDTH];

    /// @O: Current field width.
    //
    //  - Constraints
    //      - fieldWidth < FS_MAX_WIDTH
    FSInt fieldWidth;

    /// @O: Current field height.
    //
    //  - Constraints
    //      - fieldHeight < FS_MAX_HEIGHT
    FSInt fieldHeight;

    /// @E: Next available pieces.
    FSBlock nextPiece[FS_PREVIEW_MAX];

    /// @I: Buffer for calculating next pieces.
    FSBlock randomInternal[FS_RAND_BUFFER_LEN];

    /// @I: Index for `randomInternal`
    int randomInternalIndex;

    /// @E: Current pieces type.
    FSBlock piece;

    /// @E: Current pieces x position.
    FSInt x;

    /// @E: Current pieces y position.
    FSInt y;

    /// @I: Actual y position with greater precision.
    //
    // To calculate soft drop and gravity we need more precision than an
    // integer can provide.
    //
    //  - Constraints
    //      - y == (float) actualY
    float actualY;

    /// @I: Greatest 'y' the current piece can exist at without a collision.
    FSInt hardDropY;

    /// @E: Current pieces rotation state.
    FSInt theta;

    /// @E: Number of wasted movements have occurred during the games
    //      lifetime.
    FSLong finesse;

    /// @I: Number of directional movements have been performed during this
    //     pieces lifetime.
    FSLong finessePieceDirection;

    /// @I: Number of rotational movements have been performed during this
    //     pieces lifetime.
    FSLong finessePieceRotation;

    /// @O: Milliseconds between each game logic update.
    FSInt msPerTick;

    /// @O: Milliseconds between each game draw update.
    FSLong msPerDraw;

    /// @O: Length in ms that ARE should take.
    FSLong areDelay;

    /// @I: Counter for ARE.
    FSLong areTimer;

    /// @O: Can ARE be cancelled by input
    bool areCancellable;

    /// @E: Actual game length using a high precision timer.
    //
    // The game length is usually calculated as 'msPerTick * totalTicks' but
    // this is potentially inaccurate up to (+-msPerTick). 'actualTimer' acts
    // as a reliable source to ensure the game was played at the correct speed.
    //
    // This is calculated **only** on game finish.
    FSLong actualTime;

    /// @I: Generic counter for multi-tick usage.
    FSLong genericCounter;

    /// @E: Number of ticks that have elapsed during this game.
    FSLong totalTicks;

    /// @O: Current lock reset style in use.
    FSInt lockStyle;

    /// @O: Length in ms that it should take to lock a piece.
    FSLong lockDelay;

    /// @I: Counter for locking.
    FSLong lockTimer;

    /// @O: Current rotation system being used.
    FSInt rotationSystem;

    /// @O: How many blocks a piece will fall by every ms.
    float gravity;

    /// @O: How many blocks a piece will fall by every ms when soft dropping.
    float softDropGravity;

    /// @E: Current state of the internal engine.
    FSInt state;

    /// @E: State of the game during the last frame.
    FSInt lastState;

    /// @I: Key input applied during the last logic update.
    FSInput lastInput;

    /// @O: Current randomizer in play. */
    FSInt randomizer;

    /// @I: The randomizer in use during the last game update.
    //
    // Used to determine if reinitialization of a randomizer is required.
    // This allows one to alter than randomizer mid-game.
    FSInt lastRandomizer;

    /// @O: How long the "Ready" phase countdown should last in ms
    FSLong readyPhaseLength;

    /// @O: How long the "Go" phase countdown should last in ms
    FSLong goPhaseLength;

    /// @O: Whether infinite hold is allowed during pre-game.
    bool infiniteReadyGoHold;

    /// @O: Number of preview pieces displayed.
    FSInt nextPieceCount;

    /// @I: Whether a hold can be performed.
    bool holdAvailable;

    /// @E: Current piece we are holding.
    FSBlock holdPiece;

    /// @E: Number of cleared lines during the games lifetime
    FSLong linesCleared;

    /// @E: Number of blocks placed during the games lifetime.
    FSLong blocksPlaced;

    /// @O: Target number of lines to clear during this game.
    FSLong goal;
} FSGame;

///
// A generic view of a games components.
//
// The 'FSGame' instance does not handle all the components, such as input.
// This view encapsulates all these components into one structure.
///
typedef struct FSView {
    /// Current game instance.
    FSGame *game;

    /// Current input state.
    FSControl *control;

    /// Number of draw requests made during this views lifetime.
    FSLong totalFramesDrawn;
} FSView;

///
// Reset the specified games instances.
//
// This sets its internal values as they are required for a new game, and its
// options are set to those found in 'fsDefault.h'.
//
//  - FSGame *f
//      The instance to clear.
///
void fsGameClear(FSGame *f);

///
// Perform a single game update.
//
//  - FSGame *f
//      The instance to update
//
//  - const FSInput *i
//      The input for the instance to compute.
///
void fsGameTick(FSGame *f, const FSInput *i);

///
// Convert the specified into its individual blocks.
//
//  - const FSGame *f
//      The instance which options are used
//
//  - FSInt2 *dst
//      The destination buffer to store the pieces in.
//
//      Note: **must** be greater than or equal to FS_NBP in size.
//
//  - FSInt piece
//      Type of piece to generate.
//
//  - int x
//      X coordinate of the piece
//
//  - int y
//      Y coordinate of the piece
//
//  - int theta
//      Rotation state of the piece.
//
///
void fsPieceToBlocks(const FSGame *f, FSInt2 *dst, FSInt piece, int x, int y, int theta);

#endif
