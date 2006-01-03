//
// DCA Urban Terror 3.0 Beta Interface Structures
//
// Copyright (C) 2003 G. Michael Youngblood, AImachines
// All Rights Reserved.
//

#ifndef DCACOMMON
#define DCACOMMON

#define MAX_PERCEPT_ENTITIES 16
#define MAX_WEAPON_SLOTS 16
#define MAX_ITEM_SLOTS 16
#define MAX_STRING_LENGTH 81


// Guarded commands must only occur once before another command must follow.
// Multiple repetion of these commands will not be read. Intersperse a
// DCA_DO_NOTHING command between multiple desired executions.
//
typedef enum
{
	DCA_DO_NOTHING,
	DCA_WALK_FORWARD,
	DCA_WALK_BACKWARD,
	DCA_TURN_RIGHT,
	DCA_TURN_LEFT,
	DCA_RUN_FORWARD,
	DCA_RUN_BACKWARD,
	DCA_FIRE,
	DCA_LOOK_UP,
	DCA_LOOK_DOWN,
	DCA_CENTER,
	DCA_JUMP,
	DCA_DUCK,
	DCA_STRAFE_LEFT,
	DCA_STRAFE_RIGHT,
	DCA_SELECT_ITEM,                 // Guarded command
	DCA_DROP_ITEM,                   // Guarded command
	DCA_STOP,
	DCA_SPRINT_FORWARD,
	DCA_SPRINT_BACKWARD,
	DCA_RELOAD,                      // Guarded command
	DCA_BANDAGE,                     // Guarded command
	DCA_USE,                         // Guarded command
	DCA_ZOOM_IN,                     // Guarded command
	DCA_ZOOM_OUT,                    // Guarded command
	DCA_ZOOM_RESET,                  // Guarded command
	DCA_WPN_MODE,                    // Guarded command
	DCA_PREVIOUS_WPN,                // Guarded command
	DCA_NEXT_WPN,                    // Guarded command
	DCA_DROP_WPN,                    // Guarded command
	DCA_NEXT_ITEM,                   // Guarded command
	DCA_PREVIOUS_ITEM,               // Guarded command
	DCA_USE_ITEM,                    // Guarded command
	DCA_CHAT,                        // Guarded command
	DCA_TEAM_CHAT,                   // Guarded command
	DCA_TARGET_CHAT,                 // Guarded command
	//
	// Control Effector Interface 
	//
	DCA_CONTROL_MAP,                 // Guarded command
	DCA_CONTROL_QUIT,                // Guarded command
	DCA_CONTROL_SUICIDE,             // Guarded command
	DCA_CONTROL_EXEC,                // Guarded command
	DCA_CONTROL_RESTORE_HEALTH,      // Guarded command
	DCA_CONTROL_GOD,                 // Guarded command
	DCA_CONTROL_NOCLIP,              // Guarded command
	DCA_CONTROL_NOTARGET,            // Guarded command
	DCA_CONTROL_CONNECT,             // Guarded command
	DCA_CONTROL_SCREENSHOT,          // Guarded command
    DCA_CONTROL_RECORD,              // Guarded command
    DCA_CONTROL_STOPRECORD,          //	Guarded command
	DCA_CONTROL_COMMAND,             // Guarded command -- ANY CVAR Command!!
	NUM_OF_EFFECTOR_COMMANDS
} effectorCommands;

// Effector Enumerations -------------------------------------------------------
//
typedef enum
{
	EFFECTOR_PRE_COUNTER,
	EFFECTOR_AMOUNT_1,
	EFFECTOR_AMOUNT_2,
	EFFECTOR_AMOUNT_3,
	EFFECTOR_AMOUNT_4,	
	NUM_OF_FLOAT_EFFECTORS
} effectorFloatTypes;

typedef enum
{
	EFFECTOR_COMMAND_1,
	EFFECTOR_COMMAND_2,
	EFFECTOR_COMMAND_3,
	EFFECTOR_COMMAND_4,	
	EFFECTOR_POST_COUNTER,
	NUM_OF_INT_EFFECTORS
} effectorIntTypes;


// Percept Enumerations --------------------------------------------------------
//
typedef enum
{
	PERCEPT_PRE_COUNTER,
	PERCEPT_X,
	PERCEPT_Y,
	PERCEPT_Z,
	PERCEPT_PITCH,
	PERCEPT_YAW,
	PERCEPT_ROLL,
	PERCEPT_VELOCITY_X,
	PERCEPT_VELOCITY_Y,
	PERCEPT_VELOCITY_Z,
	PERCEPT_DAMAGE_FROM_X,
	PERCEPT_DAMAGE_FROM_Y,
	PERCEPT_DAMAGE_FROM_Z,
	PERCEPT_BLEED_RATE,
	NUM_OF_FLOAT_PERCEPTS
} perceptFloatTypes;

typedef enum
{
	PERCEPT_HEALTH,
	PERCEPT_TAKE_DAMAGE,
	PERCEPT_DAMAGE_FROM_WORLD,
	PERCEPT_HEAD_DAMAGE,
	PERCEPT_TORSO_DAMAGE,
	PERCEPT_BELOW_WAIST_DAMAGE,
	PERCEPT_ACCURACY_SHOTS,
	PERCEPT_ACCURACY_HITS,
	PERCEPT_DUCKED,
	PERCEPT_LADDER,
	PERCEPT_JUMPING,
	PERCEPT_LIMPING,
	PERCEPT_FIRING,
	PERCEPT_BANDAGING,
	PERCEPT_BLEEDING,
	PERCEPT_RELOAD,
	PERCEPT_USING,
	PERCEPT_VIEW_HEIGHT,
	PERCEPT_SPEED,
	PERCEPT_ENTITY_LIST_SIZE,
	PERCEPT_WEAPON_LIST_SIZE,
	PERCEPT_ITEM_LIST_SIZE,
	PERCEPT_POST_COUNTER,
	NUM_OF_INT_PERCEPTS
} perceptIntTypes;

typedef enum
{
	ENTITY_X,
	ENTITY_Y,
	ENTITY_Z,
	ENTITY_PITCH,
	ENTITY_YAW,
	ENTITY_ROLL,
	NUM_OF_FLOAT_ENTITY
} entityFloatTypes;

typedef enum
{
	ENTITY_PHYSICS_OBJECT,
	ENTITY_HEALTH,
	ENTITY_BREAK_TYPE,
	ENTITY_SWITCH_TYPE,
	NUM_OF_INT_ENTITY
} entityIntTypes;

// Structure types
//
typedef struct
{
	char  classname[MAX_STRING_LENGTH];
	float floats[NUM_OF_FLOAT_ENTITY];
	int   ints[NUM_OF_INT_ENTITY];
} entityShm;


// Urban Terror Weapon Types
//
//	UT_WP_NONE = 0,
//  UT_WP_KNIFE,
//	UT_WP_BERETTA,
//	UT_WP_DEAGLE,
//	UT_WP_SPAS12,
//	UT_WP_MP5K,
//	UT_WP_UMP45,
//	UT_WP_HK69,
//	UT_WP_M4,
//	UT_WP_G36,
//	UT_WP_PSG1,
//	UT_WP_GRENADE_HE,
//	UT_WP_GRENADE_FLASH,
//	UT_WP_GRENADE_SMOKE,
//	UT_WP_SR8,
//	UT_WP_AK103,
//	UT_WP_BOMB,
//	UT_WP_NEGEV,
//	UT_WP_GRENADE_FRAG,
//	UT_WP_NUM_WEAPONS,
//	UT_WP_KICK,
//	UT_WP_KNIFE_THROWN
//
typedef struct
{
	int id;
	int ammo;
	int clips;
	int mode;
} weaponShm;


// Urban Terror Item Types
//
// 	UT_ITEM_NONE,
//
//	UT_ITEM_REDFLAG,
//	UT_ITEM_BLUEFLAG,
//	UT_ITEM_NEUTRALFLAG,

//	UT_ITEM_KNIFE,					//	'E'
//	UT_ITEM_BERETTA,				//  'F'
//	UT_ITEM_DEAGLE,					//  'G'
//	UT_ITEM_SPAS12,					//  'H'
//	UT_ITEM_MP5K,					//  'I'
//	UT_ITEM_UMP45,					//  'J'
//	UT_ITEM_HK69,					//  'K'
//	UT_ITEM_M4,						//  'L'
//	UT_ITEM_G36,					//  'M'
//	UT_ITEM_PSG1,					//  'N'
//
//	UT_ITEM_GRENADE_HE,				//  'O'
//	UT_ITEM_GRENADE_FLASH,			//  'P'
//	UT_ITEM_GRENADE_SMOKE,			//  'Q'
//
//
//	UT_ITEM_VEST,					//  'R'
//	UT_ITEM_NVG,					//  'S'
//	UT_ITEM_MEDKIT,					//  'T'
//	UT_ITEM_SILENCER,				//  'U'
//	UT_ITEM_LASER,					//  'V'
//	UT_ITEM_HELMET,					//  'W'
//	UT_ITEM_AMMO,					//  'X'
//	UT_ITEM_APR,					//	'Y'
//
//	UT_ITEM_SR8,					//  'Z'
//
//	UT_ITEM_BUFFER0,
//	UT_ITEM_BUFFER1,
//	UT_ITEM_BUFFER2,
//	UT_ITEM_BUFFER3,
//	UT_ITEM_BUFFER4,
//	UT_ITEM_BUFFER5,
//
//	UT_ITEM_AK103,                                  //  'a'
//	UT_ITEM_BOMB,									//  'b'
//	UT_ITEM_NEGEV,									//	'c'
//	UT_ITEM_GRENADE_FRAG,							//	'd'
//
//	UT_ITEM_MAX,
//
//	UT_ITEM_C4,
//
typedef struct
{
	int id;
	int flag;                               //  currently can only be ON of OFF
} itemShm;

typedef struct
{
	float floats[NUM_OF_FLOAT_EFFECTORS];
	int   ints[NUM_OF_INT_EFFECTORS];
	char  chars[MAX_STRING_LENGTH];
	char  name[MAX_STRING_LENGTH];
	char  command[MAX_STRING_LENGTH];
} effectorShm;

typedef struct
{
	float      floats[NUM_OF_FLOAT_PERCEPTS];
	int        ints[NUM_OF_INT_PERCEPTS];
	weaponShm  weapons[MAX_WEAPON_SLOTS];
	itemShm    items[MAX_ITEM_SLOTS];
	entityShm  entities[MAX_PERCEPT_ENTITIES];
	char       map[MAX_STRING_LENGTH];
	char       netname[MAX_STRING_LENGTH];
} perceptShm;

#endif
