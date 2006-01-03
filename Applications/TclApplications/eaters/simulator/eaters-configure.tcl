### boardX and boardY define the dimensions of the *playable* eater area.
### This areas is entirely surrounded by a wall, so the actual playing map
### will be incremented by 2 in each direction.
global boardX boardY
set boardX 15
set boardY 15




### The maximum radius of grid squares that an eater can sense.
global maxSensorRange
set maxSensorRange 2

global sensorRange
set sensorRange 2

### The different possible colors that eaters can assume
global possibleEaterColors
set possibleEaterColors {red blue yellow green orange purple black}


### worldCount keeps track of the number of turns in the current game.
### worldCountLimit specifies when the game ends.
global worldCount worldCountLimit
set worldCount 0
set worldCountLimit 2000



global sendMoveCount
set sendMoveCount 0



### tickDelay is the actual number of milliseconds to wait in between
### simulation cycles (in case things are going too fast).
global tickDelay
set tickDelay 0

### ticksPerMove is the number of simulation cycles to go through between
### each move of the eaters.  This is designed to give the world a chance to
### update, and the eaters a chance to think between moves.  The only
### thing that changes in the world during a "non-moving tick" is the
### eaters' mouths.  As far as the agents are concerned, the world only
### changes on once every $ticksPerMove simulation cycles.
global ticksPerMove
set ticksPerMove 1

### soarTimePerTick and soarTimeUnit specify the number and units of
### reasoning cycles that each eater gets during a single simulation cycle.
### This is independent of simulation processing.  For example, if
### soarTimePerTick is 2 and soarTimeUnit is d, then each eater gets two
### decision cycles for every "tick" of the simulation clock.
global soarTimePerTick soarTimeUnit
set soarTimePerTick 1
set soarTimeUnit d

### ticksPerEaterCycle is simply to set how fast the eaters open and close
### their mouths.  This variable specifies how many ticks it takes for
### an eater to do a complete cycle of opening and closing.  It has no
### effect on the state of the world as the eater agents see it.
global ticksPerEaterCycle
set ticksPerEaterCycle 10

### eaterOpenMouth just sets (in degrees) how far the eaters' mouths open.
global eaterOpenMouth
set eaterOpenMouth 60

### The size, in pixels, of each grid square in the eater map
global gridSize
set gridSize 25

### The size, in pixels, of each grid square in the sensor display
global sensorGridSize
set sensorGridSize 20

### These numbers are just to specify what a "bump" looks like visually
### on the map.  It is basically an 8-pointed star and the two radius
### numbers specify the dimensions of the star points.
global bumpOuterRadius bumpInnerRadius
set bumpOuterRadius 25
set bumpInnerRadius 8

### Various color parameters that can be tailored to your heart's content
global boardColor normalfoodColor bonusfoodColor foodEdgeColor \
       wallColor wallEdgeColor eaterEdgeColor bumpColor
set boardColor gray
set normalfoodColor blue
set bonusfoodColor red
set foodEdgeColor black
set wallColor black
set wallEdgeColor gray
set eaterEdgeColor black
set bumpColor red

### Parameters specifying how big food and eaters are relative to the
### size of a map grid square
global foodSize eaterSize
set foodSize 0.3
set eaterSize 0.75


### The score values for different types of food in the game
global normalfoodScore bonusfoodScore
set normalfoodScore 5
set bonusfoodScore 10

### jumpPenalty is the amount of score an eater must "pay" in order to
### "jump" instead of doing a normal move.
global jumpPenalty
set jumpPenalty 5

### boardWalls is the default number of walls to place on the map.
global boardWalls
set boardWalls 10
