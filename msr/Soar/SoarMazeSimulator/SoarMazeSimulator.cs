//------------------------------------------------------------------------------
// SoarMazeSimulator.cs
//
// Original version written by Ben Axelrod and Trevor Taylor
// http://www.soft-tech.com.au/MSRS/
//
// Updated for use with Soar by Jonathan Voigt
//------------------------------------------------------------------------------

using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.Permissions;
using xml = System.Xml;

// Added for Color definition
using System.Drawing;

#region Simulation namespaces
using Microsoft.Robotics.Simulation;
using Microsoft.Robotics.Simulation.Engine;
using engineproxy = Microsoft.Robotics.Simulation.Engine.Proxy;
using Microsoft.Robotics.Simulation.Physics;
using drive = Microsoft.Robotics.Services.Simulation.Drive.Proxy;
using lrf = Microsoft.Robotics.Services.Simulation.Sensors.LaserRangeFinder.Proxy;
using bumper = Microsoft.Robotics.Services.Simulation.Sensors.Bumper.Proxy;
using simwebcam = Microsoft.Robotics.Services.Simulation.Sensors.SimulatedWebcam.Proxy;
using Microsoft.Robotics.PhysicalModel;
using System.ComponentModel;
#endregion

using Microsoft.Robotics.Simulation.Physics;
using Microsoft.Robotics.PhysicalModel;


namespace Robotics.SoarMazeSimulator
{
    [DisplayName("SoarMazeSimulator")]
    [Description("The SoarMazeSimulator Service")]
    [Contract(Contract.Identifier)]
    ////IMPORTANT
    public class SoarMazeSimulatorService : DsspServiceBase
    {
        #region setup

        public const string InitialStateUri = ServicePaths.MountPoint + @"/Apps/Soar/Config/SoarMazeSimulator.Config.xml";

        // Add an InitialStatePartner so that the config file will be read
        // NOTE: Creating a new instance of the state here will NOT
        // work if there is no config file because InitialStatePartner
        // will replace it with null!!! See the code in Start().
        [InitialStatePartner(Optional = true, ServiceUri = InitialStateUri)]
        private SoarMazeSimulatorState _state = null;

        // Added MassMap and created a local copy of HeightMap
        // in case there were not enough items in the State.
        // Added a flag as well to create a sphere instead of a box.
        private string[] _WallTextures = new string[16];
        private Vector3[] _WallColors = new Vector3[16];
        private float[] _WallHeights = new float[16];
        private float[] _WallMasses = new float[16];
        private bool[] _UseSphere = new bool[16];

        // Physics engine instance
        PhysicsEngine _physicsEngine;
        // Port used to communicate with simulation engine service directly, no cloning
        SimulationEnginePort _simEnginePort;

        // partner attribute will cause simulation engine service to start
        [Partner("Engine",
            Contract = engineproxy.Contract.Identifier,
            CreationPolicy = PartnerCreationPolicy.UseExistingOrCreate)]
        private engineproxy.SimulationEnginePort _engineServicePort = new engineproxy.SimulationEnginePort();

        [ServicePort("/soarmazesimulator", AllowMultipleInstances=false)]
        private SoarMazeSimulatorOperations _mainPort = new SoarMazeSimulatorOperations();

        /// <summary>
        /// Default Service Constructor
        /// </summary>
        public SoarMazeSimulatorService(DsspServiceCreationPort creationPort) : base(creationPort)
        {

        }

        #region properties

        static int WallCellColorThresh = -10000000;    // pixels less than this value will be counted as walls

        static bool OptimizeBlocks = true;  //can significantly reduce number of entities and thus increase frames per second
        int BlockCounter = 0;               //to count blocks after optimization

        //++ TT
        // Added for better use of bitmaps
        static bool UseBackgroundColor = true;
        static bool CenterMaze = true;

        // Simple colors
        //
        // Anyone remember CGA?
        // You don't have to list all the values in an enum,
        // but it looks pretty
        //
        public enum BasicColor : byte
        {
            Black       = 0,
            Red         = 1,
            Lime        = 2,
            Yellow      = 3,
            Blue        = 4,
            Magenta     = 5,
            Cyan        = 6,
            White       = 7,
            DarkGrey    = 8,
            Maroon      = 9,
            Green       = 10,
            Olive       = 11,
            Navy        = 12,
            Purple      = 13,
            Cobalt      = 14,
            Grey        = 15
        }

        //-- TT

        #endregion

        /// <summary>
        /// Service Start
        /// </summary>
        protected override void Start()
        {
            int i;

            // The state might already have been created using
            // the Initial State Partner above. If so, then we
            // don't want to create a new one!
            if (_state == null)
            {
                _state = new SoarMazeSimulatorState();
                // Do any other initialization here for the default
                // settings that you might want ...
            }

            // TT Feb-2007 - Shock! Horror! Setting the maze in the config
            // file did not work because it was initialized in the constructor
            // for the State!
            // The maze here is one with lots of different objects including some balls
            // Note the path - It is relative to where dsshost is started from
            // Other samples are:
            // ModelSmall.gif -- A smaller model than the one above
            // office.bmp -- A black and white image of an "office" maze
            // Jul-2007:
            // Changed the location of the files
            if (_state.Maze == null || _state.Maze == "")
                _state.Maze = "Apps/Soar/SoarMazeSimulator/ModelLarge.bmp";

            // Make sure that there is a floor texture
            // Plenty of others to try, e.g. concrete.jpg.
            if (_state.GroundTexture == null || _state.GroundTexture == "")
                _state.GroundTexture = "cellfloor.jpg";

            // TT Dec-2006 - This is a fudge to support upgrading from
            // prior versions where the RobotType did not exist. When
            // MSRS loades the config file, it does not populate any
            // of the fields that are missing. Therefore the RobotType
            // is null and this causes the code to crash later on.
            if (_state.RobotType == null)
                _state.RobotType = "Pioneer3DX";

            // Now initialize our internal copies of state info
            // This is a little bit of insurance against a bad
            // config file ...
            // Copy as many textures as available up to the max
            for (i = 0; (i < 16) && (i < _state.WallTextures.Length); i++)
            {
                _WallTextures[i] = _state.WallTextures[i];
            }
            // Fill any remaining textures with empty string
            for ( ; i < 16; i++)
                _WallTextures[i] = "";

            // Copy as many colors as specified
            // NOTE: The constructor for the State sets all of the
            // colors to the standard ones, so any that are not
            // specified will default to them.
            for (i = 0; (i < 16) && (i < _state.WallColors.Length); i++)
            {
                _WallColors[i] = _state.WallColors[i];
            }
            // Fill any remaining colors with the defaults
            for (; i < 16; i++)
                _WallColors[i] = SoarMazeSimulatorState.DefaultColors[i];

            // Copy as many heights as specified
            for (i = 0; (i < 16) && (i < _state.HeightMap.Length); i++)
            {
                _WallHeights[i] = _state.HeightMap[i];
            }
            // Fill any remaining heights with the defaults
            for (; i < 16; i++)
                _WallHeights[i] = 5.0f;

            // Copy as many weights as specified
            for (i = 0; (i < 16) && (i < _state.MassMap.Length); i++)
            {
                _WallMasses[i] = _state.MassMap[i];
            }
            // Fill any remaining weights with the defaults
            for (; i < 16; i++)
                _WallMasses[i] = 0.0f;

            // Copy as many sphere flags as specified
            for (i = 0; (i < 16) && (i < _state.UseSphere.Length); i++)
            {
                _UseSphere[i] = _state.UseSphere[i];
            }
            // Fill any remaining flags with false
            for (; i < 16; i++)
                _UseSphere[i] = false;

            if (_state.SphereScale <= 0.0f)
                _state.SphereScale = 1.0f;

            if (_state.HeightScale <= 0.0f)
                _state.HeightScale = 1.0f;

            // Copy back our private versions which might have the
            // effect of extending the state
            _state.WallColors = _WallColors;
            _state.WallTextures = _WallTextures;
            _state.HeightMap = _WallHeights;
            _state.MassMap = _WallMasses;
            _state.UseSphere = _UseSphere;

            // Now save the State
            // This creates a new file the first time it is run
            // Later, it re-reads the existing file, but by then
            // the file has been populated with the defaults
            SaveState(_state);

            // Listen on the main port for requests and call the appropriate handler.
            ActivateDsspOperationHandlers();

            // Publish the service to the local Node Directory
            DirectoryInsert();

			// display HTTP service Uri
			LogInfo(LogGroups.Console, "Service uri: ");

            // Cache references to simulation/rendering and physics
            _physicsEngine = PhysicsEngine.GlobalInstance;
            _simEnginePort = SimulationEngine.GlobalInstancePort;

            // TT Dec-2006 - Set up the initial camera view
            SetupCamera();

            // Add objects (entities) in our simulated world
            PopulateWorld();
        }

        // TT Dec-2006 - Copied from Sim Tutorial 2 in V1.0
        private void SetupCamera()
        {
            // Set up initial view
            CameraView view = new CameraView();
//            view.EyePosition = new Vector3(0.0f, 6.41f, -5.41f);
//            view.LookAtPoint = new Vector3(0.0f, 5.88f, -4.81f);
            // TT Jul-2007 - Move back a little to see more of the field
            // Note that these values angle the camera down at 45 degrees
            // looking along the Z axis
            view.EyePosition = new Vector3(0.0f, 7.0f, -7.0f);
            view.LookAtPoint = new Vector3(0.0f, 5.0f, -5.0f);
            _simEnginePort.Update(view);
        }

        #endregion

        #region Basic World

        // Build the entire world view in the simulator
        private void PopulateWorld()
        {
            AddSky();
            AddGround();
            AddMaze();
            AddRobot();
        }

        void AddSky()
        {
            // Add a sky using a static texture. We will use the sky texture
            // to do per pixel lighting on each simulation visual entity
            SkyEntity sky = new SkyEntity("sky.dds", "sky_diff.dds");
            _simEnginePort.Insert(sky);
        }

        // The name says it all ...
        void AddGround()
        {
            HeightFieldShapeProperties hf = new HeightFieldShapeProperties("height field",
                64,     // number of rows 
                100,    // distance in meters, between rows
                64,     // number of columns
                100,    // distance in meters, between columns
                1,      // scale factor to multiple height values 
                -1000); // vertical extent of the height field. Should be set to large negative values

            // create array with height samples
            hf.HeightSamples = new HeightFieldSample[hf.RowCount * hf.ColumnCount];
            for (int i = 0; i < hf.RowCount * hf.ColumnCount; i++)
            {
                hf.HeightSamples[i] = new HeightFieldSample();
                hf.HeightSamples[i].Height = (short)(Math.Sin(i * 0.01));
            }

            // create a material for the entire field. We could also specify material per sample.
            hf.Material = new MaterialProperties("ground", 0.8f, 0.5f, 0.8f);

            // insert ground entity in simulation and specify a texture
            _simEnginePort.Insert(new HeightFieldEntity(hf, _state.GroundTexture));
        }

        #endregion

        #region Maze

        // TT --
        // Converted the maze to a height array instead of booleans
        // Added an offset so that the maze can be centered in world coords
        // Added color for the walls using a texture, which is quite
        // flexible because it does not require recompiling the code.
        //
        // This is tacky -- Last minute change to get the correct
        // coordinates of the image color inside AddWall(). Should
        // be handled better ... Slap on the wrist!
        int xOffset, yOffset;

        void AddMaze()
        {
            // TT - float, not bool
            float[,] maze = ParseImage(_state.Maze);
            int height = maze.GetLength(1);
            int width = maze.GetLength(0);
            // TT
            int[,] counters = new int[width, height];
            int tempX1, tempY1;
            int tempX2, tempY2;
            BasicColor PixelColor;

            // TT -- Allow centering of the maze
            // (Easier to find in the simulator! However, you can't
            // as easily use the pixel coordinates to figure out where
            // things are.)
            if (CenterMaze)
            {
                xOffset = (int)((-width / 2.0) + 0.5);
                yOffset = (int)((-height / 2.0) + 0.5);
            }
            else
            {
                xOffset = 0;
                yOffset = 0;
            }

            if (OptimizeBlocks)
            {
                int count;
                int thisRowCount;
                float currentHeight;

                //merge horizontal blocks
                for (int y = 0; y < height; y++)
                {
                    int x = 0;
                    while (x < width - 1)
                    {
                        //at least 2 blocks to merge
                        if (maze[x, y] > 0.0f && maze[x + 1, y] > 0.0f)
                        {
                            int startX = x;
                            // TT -- Only merge wall segments of the same height
                            count = 0;
                            currentHeight = maze[x, y];
                            while (x < width && maze[x, y] > 0.0f
                                && maze[x, y] == currentHeight)
                            {
                                maze[x, y] = 0.0f;
                                counters[x, y] = 0;
                                x++;
                                count++;
                            }
                            // TT -- Just mark the map, don't draw anything here
                            counters[startX, y] = count;
                            maze[startX, y] = currentHeight;
                        }
                        else
                        {
                            if (maze[x, y] > 0.0f)
                                counters[x, y] = 1;
                            else
                                counters[x, y] = 0;
                            x++;
                        }
                    }
                    if (x < height)
                    {
                        if (maze[x, y] > 0.0f)
                            counters[x, y] = 1;
                        else
                            counters[x, y] = 0;
                    }
                }

                //merge remaining vertical blocks
                for (int x = 0; x < width; x++)
                {
                    int y = 0;
                    while (y < height - 1)
                    {
                        //at least 2 blocks to merge
                        // Must have the same row count AND height
                        if (counters[x, y] > 0 && counters[x, y + 1] == counters[x, y]
                            && maze[x, y] == maze[x, y + 1])
                        {
                            int startY = y;
                            count = 0;
                            thisRowCount = counters[x, y];
                            // TT -- Only merge wall segments of the same height
                            currentHeight = maze[x, y];
                            while (y < height && counters[x, y] == thisRowCount
                                && maze[x, y] == currentHeight)
                            {
                                maze[x, y] = 0.0f;
                                counters[x, y] = 0;
                                y++;
                                count++;
                            }
                            // TT -- Add offset
                            tempY1 = startY + yOffset;
                            tempX1 = x + xOffset;
                            tempY2 = startY + count - 1 + yOffset;
                            tempX2 = x + thisRowCount - 1 + xOffset;
                            PixelColor = ParseColor(img.GetPixel(x, startY));
                            AddWall(tempY1, tempX1, tempY2, tempX2, currentHeight, PixelColor);
                        }
                        else
                        {
                            if (counters[x, y] > 0)
                            {
                                tempY1 = y + yOffset;
                                tempX1 = x + xOffset;
                                tempY2 = y + yOffset;
                                tempX2 = x + counters[x, y] - 1 + xOffset;
                                PixelColor = ParseColor(img.GetPixel(x, y));
                                AddWall(tempY1, tempX1, tempY2, tempX2, maze[x, y], PixelColor);
                                maze[x, y] = 0.0f;
                                counters[x, y] = 0;
                            }

                            y++;
                        }
                    }
                    // TT -- Boundary condition
                    if (y < height)
                    {
                        if (counters[x, y] > 0)
                        {
                            tempY1 = y + yOffset;
                            tempX1 = x + xOffset;
                            tempY2 = y + yOffset;
                            tempX2 = x + counters[x, y] - 1 + xOffset;
                            PixelColor = ParseColor(img.GetPixel(x, y));
                            AddWall(tempY1, tempX1, tempY2, tempX2, maze[x, y], PixelColor);
                            maze[x, y] = 0.0f;
                            counters[x, y] = 0;
                        }
                    }
                }
            }

            //draw all blocks left over
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    if (maze[x, y] > 0.0f)
                    {
                        // TT -- Add offset
                        tempY1 = y + yOffset;
                        tempX1 = x + xOffset;
                        PixelColor = ParseColor(img.GetPixel(x, y));
                        AddWall(tempY1, tempX1, maze[x, y], PixelColor);
                        // This is only for debugging
                        // All blocks should be zero at the end ...
                        maze[x, y] = 0.0f;
                    }
                }
            }

            if (OptimizeBlocks)
            {
                Console.WriteLine("\nOptimization reduced number of wall blocks to: " + BlockCounter +"\n");
            }

        }


        // TT -- Simple fuzzy color parsing into the 16 basic colors
        const int BlackThreshold = 16;
        const int WhiteThreshold = 208;
        const int ColorThreshold = 128;
        const double GreyTolerance = 0.10;

        // This overload is not really necessary, but I'm lazy
        BasicColor ParseColor(Color pixel)
        {
            return ParseColor(pixel.R, pixel.G, pixel.B);
        }

        BasicColor ParseColor(int r, int g, int b)
        {
            int rgbSum = r + g + b;

            // Sort out Black and White straight away.
            // The assumption here is that sufficiently
            // intense or dark colors are just white or black.
            if (rgbSum < BlackThreshold * 3)
                return BasicColor.Black;
            else if (rgbSum > WhiteThreshold * 3)
                return BasicColor.White;
            else
            {
                // Next check for grey
                // This compares the range of values to see if they
                // are basically all equal, i.e. no dominant color = grey
                float normMax, normMin, normRange;
                int valMax, valMin;
                valMax = Math.Max(r, g);
                valMax = Math.Max(valMax, b);
                valMin = Math.Min(r, g);
                valMin = Math.Min(valMin, b);
                normMax = (float)valMax;
                normMin = (float)valMin;
				normMax /= rgbSum;
				normMin /= rgbSum;
				normRange = normMax - normMin;
                if (normRange < GreyTolerance)
                {
                    if (normMax >= 160)
                        return BasicColor.Grey;
                    else
                        return BasicColor.DarkGrey;
                }

                // Now we have a more complicated task
                // but it is made easier by the definition
                // of BasicColor
                byte color = 0;

                // Check for dark versions of the colors
                if (valMax < 160)
                {
                    color += 8;
                    if (r >= ColorThreshold/2)
                        color += 1;
                    if (g >= ColorThreshold/2)
                        color += 2;
                    if (b >= ColorThreshold/2)
                        color += 4;
                }
                else
                {
                    // Now check the thresholds for normal colors
                    if (r >= ColorThreshold)
                        color += 1;
                    if (g >= ColorThreshold)
                        color += 2;
                    if (b >= ColorThreshold)
                        color += 4;
                }
                return (BasicColor)color;
            }
        }


        // Parse the image bitmap into a maze
        //
        // TT -- Modified so that you can have a background colour
        // to use for the bitmap and also to use the color as an
        // index for the height
        //
        static Bitmap img;
        float[,] ParseImage(string file)
        {
            img = (Bitmap)Image.FromFile(file);
            float[,] imgArray = new float[img.Width, img.Height];
            int WallCount = 0;

            // TT -- Allow background color to be used
            // Select the color of the top-left pixel.
            // We could be a lot smarter here. For instance, search the
            // image and find the predominant color which must be the
            // background. However, this will do for now.
            bool IsWall;
            //BasicColor Background = BasicColor.White;
            Color BackgroundColor = Color.White;
            Color PixelColor;
            BasicColor PixelBasicColor;

            if (UseBackgroundColor)
            {
                BackgroundColor = img.GetPixel(0, 0);
                //                Background = ParseColor(BackgroundColor);
            }

            for (int y = 0; y < img.Height; y++)
            {
                for (int x = 0; x < img.Width; x++)
                {
                    if (UseBackgroundColor)
                    {
                        // Get a basic pixel color
                        PixelColor = img.GetPixel(x, y);
                        PixelBasicColor = ParseColor(PixelColor);
                        if (PixelColor.R != BackgroundColor.R ||
                            PixelColor.G != BackgroundColor.G ||
                            PixelColor.B != BackgroundColor.B)
                        {
                            // Return the height at this pixel location
                            imgArray[x, y] = _WallHeights[(byte)PixelBasicColor];
                            WallCount++;
                        }
                        else
                            imgArray[x, y] = 0.0f;
                    }
                    else
                    {
                        if (img.GetPixel(x, y).ToArgb() < WallCellColorThresh)
                        {
                            imgArray[x, y] = _state.DefaultHeight;
                            WallCount++;
                        }
                        else
                            imgArray[x, y] = 0.0f;
                    }
                }
            }
            Console.WriteLine("\nAdding grid world of size " + img.Width + " x " + img.Height + ". With " + WallCount + " wall blocks.");
            return imgArray;
        }


        //TT -- Changed to include height and color
//        void AddWall(int row, int col)
//        {
//            AddWall(row, col, DefaultHeight, BasicColor.White);
//        }

        // Adds a simple cube at a specified location in the maze grid
        void AddWall(int row, int col, float height, BasicColor color)
        {

            // TT Oct-2006 - Add an option to use a sphere instead of a cube
            if (_UseSphere[(byte)color])
            {
                SphereShapeProperties cSphereShape = null;
                SingleShapeEntity sphere = null;
                float radius;

                radius = _state.SphereScale *  height / 2.0f;

                // create simple entity, with a single shape
                cSphereShape = new SphereShapeProperties(
                    // TT -- Change to infinite mass so that the walls
                    // do not "blow apart" if you bump them!
                    // TT Oct 2006 -- Allow user to specify this
                        _WallMasses[(byte)color], // mass in kilograms.
                        new Pose(),     // relative pose
                        radius);    // radius
                cSphereShape.Material = new MaterialProperties("gsphere", 1.0f, 0.01f, 0.01f);
                // Set the color of the sphere according to the bitmap image
                // or the specified color if no bitmap
                if (_WallTextures[(byte)color] == "")
                {
                    // TT - Changed for October CTP because DiffuseColor
                    // is a Vector4, but my WallColors are Vector3
                    //cBoxShape.DiffuseColor = _WallColors[(byte)color];
                    cSphereShape.DiffuseColor.X = (float)(_WallColors[(byte)color].X / 255.0);
                    cSphereShape.DiffuseColor.Y = (float)(_WallColors[(byte)color].Y / 255.0);
                    cSphereShape.DiffuseColor.Z = (float)(_WallColors[(byte)color].Z / 255.0);
                    cSphereShape.DiffuseColor.W = 1.0f;
                }
                else
                    cSphereShape.TextureFileName = _WallTextures[(byte)color];

                sphere = new SingleShapeEntity(new SphereShape(cSphereShape),
                    new Vector3((col * -_state.GridSpacing),
                            radius,
                            -(row * _state.GridSpacing)));

                // Name the entity. All entities must have unique names
                sphere.State.Name = "ball_" + row + "_" + col;

                // Insert entity in simulation.  
                _simEnginePort.Insert(sphere);
            }
            else
            {
                // Dimensions are in meters
                Vector3 dimensions =
                    new Vector3(_state.WallBoxSize * _state.GridSpacing,
                            height * _state.HeightScale,
                            _state.WallBoxSize * _state.GridSpacing);
                BoxShapeProperties cBoxShape = null;
                SingleShapeEntity box = null;

                // create simple immovable entity, with a single shape
                cBoxShape = new BoxShapeProperties(
                    // TT -- Change to infinite mass so that the walls
                    // do not "blow apart" if you bump them!
                    // TT Oct 2006 -- Allow user to specify this
                        _WallMasses[(byte)color], // mass in kilograms.
                        new Pose(),     // relative pose
                        dimensions);    // dimensions
                cBoxShape.Material = new MaterialProperties("gbox", 1.0f, 0.4f, 0.5f);
                // Set the color of the box according to the bitmap image
                // or the specified color if no bitmap
                if (_WallTextures[(byte)color] == "")
                {
                    // TT - Changed for October CTP because DiffuseColor
                    // is a Vector4, but my WallColors are Vector3
                    //cBoxShape.DiffuseColor = _WallColors[(byte)color];
                    cBoxShape.DiffuseColor.X = (float)(_WallColors[(byte)color].X / 255.0);
                    cBoxShape.DiffuseColor.Y = (float)(_WallColors[(byte)color].Y / 255.0);
                    cBoxShape.DiffuseColor.Z = (float)(_WallColors[(byte)color].Z / 255.0);
                    cBoxShape.DiffuseColor.W = 0.5f;
                }
                else
                    cBoxShape.TextureFileName = _WallTextures[(byte)color];

                box = new SingleShapeEntity(new BoxShape(cBoxShape),
                    new Vector3(col * -_state.GridSpacing,
                            height * _state.HeightScale / 2,
                            -(row * _state.GridSpacing)));

                // Name the entity. All entities must have unique names
                box.State.Name = "wall_" + row + "_" + col;

                // Insert entity in simulation.  
                _simEnginePort.Insert(box);
            }

            BlockCounter++;
        }

        // TT -- Changed to add a height and color
//        void AddWall(int startRow, int startCol, int endRow, int endCol)
//        {
//            AddWall(startRow, startCol, endRow, endCol, DefaultHeight);
//        }

        // Adds a long wall in the maze grid
        // Useful for reducing number of elements in simulation for better performance
        // TT -- Note that the existing code used height to refer to the
        // depth of the wall. Therefore the real height is called boxSize.
        void AddWall(int startRow, int startCol, int endRow, int endCol, float boxSize, BasicColor color)
        {
            int width = Math.Abs(endCol - startCol) + 1;
            int height = Math.Abs(endRow - startRow) + 1;

            float realWidth = (width * _state.GridSpacing) - (_state.GridSpacing - _state.WallBoxSize*_state.GridSpacing);
            float realHeight = (height * _state.GridSpacing) - (_state.GridSpacing - _state.WallBoxSize*_state.GridSpacing);

            //because the box is placed relative to the center of mass
            float widthOffset = (Math.Abs(endCol - startCol) * _state.GridSpacing) / 2;
            float heightOffset = -(Math.Abs(endRow - startRow) * _state.GridSpacing) / 2;

            if (_UseSphere[(byte)color])
            {
                // This object is a Sphere
                SphereShapeProperties cSphereShape = null;
                SingleShapeEntity sphere = null;
                float radius;

                radius = (float)(_state.SphereScale * Math.Sqrt(realWidth * realWidth + realHeight * realHeight) / 2.0f);

                // create simple entity, with a single shape
                cSphereShape = new SphereShapeProperties(
                    // TT -- Change to infinite mass so that the walls
                    // do not "blow apart" if you bump them!
                    // TT Oct 2006 -- Allow user to specify this
                    _WallMasses[(byte)color], // mass in kilograms.
                    new Pose(),     // relative pose
                    radius);        // radius
                cSphereShape.Material = new MaterialProperties("gsphere", 0.9f, 0.05f, 0.1f);
                cSphereShape.Material.Advanced = new MaterialAdvancedProperties();
                cSphereShape.Material.Advanced.RestitutionCombineMode = CoefficientsCombineMode.Max;
                cSphereShape.Material.Advanced.FrictionCombineMode = CoefficientsCombineMode.Min;
                cSphereShape.Material.Advanced.Spring = new SpringProperties();
                cSphereShape.Material.Advanced.Spring.SpringCoefficient = 0.9f;
                cSphereShape.Material.Advanced.Spring.DamperCoefficient = 0.1f;
                cSphereShape.Advanced = new ShapeAdvancedProperties();
                cSphereShape.Advanced.PhysicsCalculationPasses = 20.0f;
                //cSphereShape.Advanced.MassSpaceIntertiaTensor = new Vector3(1.0f, 0.0f, 1.0f);
                // TT - These do not seem to have much effect
                //cSphereShape.MassDensity.AngularDamping = 0.0f;
                //cSphereShape.MassDensity.LinearDamping = 0.0f;
                //cSphereShape.MassDensity.Mass = _WallMasses[(byte)color];
                //cSphereShape.MassDensity.Density = 0.1f;

                // Set the color of the sphere according to the bitmap image
                // or the specified color if no bitmap
                if (_WallTextures[(byte)color] == "")
                {
                    // TT - Changed for October CTP because DiffuseColor
                    // is a Vector4, but my WallColors are Vector3
                    //cBoxShape.DiffuseColor = _WallColors[(byte)color];
                    cSphereShape.DiffuseColor.X = (float)(_WallColors[(byte)color].X / 255.0);
                    cSphereShape.DiffuseColor.Y = (float)(_WallColors[(byte)color].Y / 255.0);
                    cSphereShape.DiffuseColor.Z = (float)(_WallColors[(byte)color].Z / 255.0);
                    cSphereShape.DiffuseColor.W = 1.0f;
                }
                else
                    cSphereShape.TextureFileName = _WallTextures[(byte)color];

                sphere = new SingleShapeEntity(new SphereShape(cSphereShape),
                    new Vector3((startCol * -_state.GridSpacing) - widthOffset,
                                radius*2,
                                -(startRow * _state.GridSpacing) + heightOffset)
                    );

                // Name the entity. All entities must have unique names
                sphere.State.Name = "ball_" + startRow + "_" + startCol;

                // Insert entity in simulation.  
                _simEnginePort.Insert(sphere);

                /*
                SphereShapeProperties cSphereShape = null;
                SingleShapeEntity sphere = null;
                float radius;

                radius = (float)(_state.SphereScale * Math.Sqrt(realWidth * realWidth + realHeight * realHeight) / 2.0f);

                // create simple entity, with a single shape
                cSphereShape = new SphereShapeProperties(
                        1.0f,   // mass in kilograms.
                        new Pose(),     // relative pose
                        radius);    // radius
                cSphereShape.Material = new MaterialProperties("gsphere", 1.0f, 0.01f, 0.01f);
                sphere = new SingleShapeEntity(new SphereShape(cSphereShape),
                    new Vector3((startCol * -_state.GridSpacing) - widthOffset,
                                radius,
                                -(startRow * _state.GridSpacing) + heightOffset)
                    );

                // Name the entity. All entities must have unique names
                sphere.State.Name = "ball_" + startRow + "_" + startCol;

                // Insert entity in simulation.  
                _simEnginePort.Insert(sphere);
                 */
            }
            else
            {
                // This object is a wall (stretched cube)
                Vector3 dimensions =
                    new Vector3(realWidth, boxSize * _state.HeightScale, realHeight);
                        // Dimensions are in meters
                BoxShapeProperties cBoxShape = null;
                SingleShapeEntity box = null;

                cBoxShape = new BoxShapeProperties(
                    // TT -- Change to infinite mass so that the walls
                    // do not "blow apart" if you bump them!
                    // TT Oct 2006 -- Allow user to specify this
                        _WallMasses[(byte)color], // mass in kilograms.
                        new Pose(),     // relative pose
                        dimensions);    // dimensions
//                cBoxShape = new BoxShapeProperties(0, new Pose(), dimensions);
                // Walls have the same properties as the ground
                cBoxShape.Material = new MaterialProperties("gbox", 0.8f, 0.5f, 0.8f);
                // Set the color of the box according to the bitmap image
                // or the specified color if no bitmap
                if (_WallTextures[(byte)color] == "")
                {
                    // TT - Changed for October CTP because DiffuseColor
                    // is a Vector4, but my WallColors are Vector3
                    //cBoxShape.DiffuseColor = _WallColors[(byte)color];
                    cBoxShape.DiffuseColor.X = (float)(_WallColors[(byte)color].X / 255.0);
                    cBoxShape.DiffuseColor.Y = (float)(_WallColors[(byte)color].Y / 255.0);
                    cBoxShape.DiffuseColor.Z = (float)(_WallColors[(byte)color].Z / 255.0);
                    cBoxShape.DiffuseColor.W = 0.5f;
                }
                else
                    cBoxShape.TextureFileName = _WallTextures[(byte)color];

                box = new SingleShapeEntity(new BoxShape(cBoxShape),
                    new Vector3((startCol * -_state.GridSpacing) - widthOffset,
                                boxSize * _state.HeightScale / 2,
                                -(startRow * _state.GridSpacing) + heightOffset)
                    );
                // Name the entity. All entities must have unique names
                box.State.Name = "wall_" + startRow + "_" + startCol;
                _simEnginePort.Insert(box);
            }

            BlockCounter++;
        }

        #endregion

        #region Robot Entities

        // TT Dec-2006 - Select the type of robot based on the setting
        // in the config file. Currently only two types are supported.
        void AddRobot()
        {
            // Note that the new robot is created just slightly off the
            // ground so that it "drops in". This is not necessary ...
            Vector3 position = new Vector3(_state.RobotStartCellCol * -_state.GridSpacing,
                                0.05f,
                                -(_state.RobotStartCellRow * _state.GridSpacing));
            if (_state.RobotType.ToLower() == "pioneer3dx")
                AddPioneer3DXRobot(position);
            else if (_state.RobotType.ToLower() == "legonxt")
                AddLegoNxtRobot(position);
            else
                AddPioneer3DXRobot(position);
        }

        #region Pioneer

        void AddPioneer3DXRobot(Vector3 position)
        {
            // TT Dec-2006 - Make the position a parameter
            // TT Jun-2007 - Changes to the Simulated Differential Drive
            // required a new version of the Pioneer3DX. This is annoying
            // because it was only a change of class name but it had some
            // flow-on effects. The problem arises because you can't just
            // replace an entity that is in built into the Simulator.
            TTPioneer3DX robotBaseEntity = CreateMotorBase(ref position);

            // Create Laser entity and start simulated laser service
            LaserRangeFinderEntity laser = CreateLaserRangeFinder();

            // Add laser as child to motor base
            robotBaseEntity.InsertEntity(laser);

            // Create bumper array entity and start simulated bumper service
            BumperArrayEntity bumperArray = CreateBumperArray();

            // Insert as child of motor base
            robotBaseEntity.InsertEntity(bumperArray);

            // TT - Copied from Oct CTP Simulation Tutorial 2
            // Create Camera Entity and start SimulatedWebcam service
            CameraEntity camera = CreateCamera();
            // insert as child of motor base
            robotBaseEntity.InsertEntity(camera);

            // Reverse the orientation of the robot so that it will
            // drive "forwards" from the user's perspective (it is
            // actually driving in the Z direction)
            robotBaseEntity.State.Pose.Orientation = Quaternion.FromAxisAngle(0, 1, 0, (float)(Math.PI));

            // Finaly insert the motor base and its two children 
            // to the simulation
            _simEnginePort.Insert(robotBaseEntity);

        }

        // TT Jun-2007 - Changed the return type because it uses a modified
        // version of the Pioneer3DX entity to support RotateDegrees
        private TTPioneer3DX CreateMotorBase(ref Vector3 position)
        {
            // use supplied entity that creates a motor base 
            // with 2 active wheels and one caster
            // TT Jun-2007 - See comment above
            TTPioneer3DX robotBaseEntity = new TTPioneer3DX(position);

            // specify mesh. 
            // TT - October CTP no longer uses .x files
            //robotBaseEntity.State.Assets.Mesh = "archos_01.x";
            robotBaseEntity.State.Assets.Mesh = "Pioneer3dx.bos";
            // TT - From Oct CTP
            // Specify color if no mesh is specified. 
            robotBaseEntity.ChassisShape.State.DiffuseColor = new Vector4(0.8f, 0.25f, 0.25f, 1.0f);

            // the name below must match manifest
            // Really??? It's not in the manifest!
            // TT Dec-2006 - Remove the leading slash.
            // In all the CTPs it worked with the slash, but not in V1.0.
            robotBaseEntity.State.Name = "P3DXMotorBase";

            // Start simulated arcos motor service
            // Note: The name of the service is taken from the Namespace,
            // which is why we have used ...Drive.Simulated.Proxy
            // The Entity Partner service that is created must have the
            // same name as the entity itself. This has nothing to do with
            // the name of the underlying "hardware" service.
            // This simulation entity is called "/MotorBase" and a new
            // service will be created with this name. It will be of
            // type Drive.Simulated.Proxy.Contract.Identifier which in
            // turn has a service port name of /SimulatedDifferentialDrive.
            //
            // TT Dec-2006 - This code has changed subtly in V1.0 Sim Tutorial 2.
            // Notice also that the slash is now in the literal.
            //CreateService(
            //    drive.Contract.Identifier,
            //    Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
            //        "http://localhost" + robotBaseEntity.State.Name)
            //);
            drive.Contract.CreateService(ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + robotBaseEntity.State.Name)
            );
            return robotBaseEntity;
        }

        private LaserRangeFinderEntity CreateLaserRangeFinder()
        {
            // Create a Laser Range Finder Entity.
            // Place it 30cm above base CenterofMass. 
            LaserRangeFinderEntity laser = new LaserRangeFinderEntity(
                new Pose(new Vector3(0, 0.30f, 0)));
            // TT Dec-2006 - Remove leading slash for V1.0
            laser.State.Name = "P3DXLaserRangeFinder";
            // TT - From Oct CTP
            laser.LaserBox.State.DiffuseColor = new Vector4(0.25f, 0.25f, 0.8f, 1.0f);

            // Create LaserRangeFinder simulation service and specify
            // which entity it talks to
            CreateService(
                lrf.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + laser.State.Name));
            return laser;
        }

        private BumperArrayEntity CreateBumperArray()
        {
            // Create a bumper array entity with two bumpers
            // TT -- I changed the thickness of the bumpers to try to
            // reduce the problem where both front and back seem
            // to trigger when it "climbs" a wall. Moving the
            // bumper up higher on the robot did not work because
            // it simply got stuck on the wall!
            // TT Jul-2007 - There is some problem with the bumpers
            // not being recognised during collisions. Put them back
            // the way they were before.
            BoxShape frontBumper = new BoxShape(
                new BoxShapeProperties("front",
                    0.001f,
                    new Pose(new Vector3(0, 0.05f, -0.25f)),
                    new Vector3(0.40f, 0.03f, 0.03f)
                    //new Vector3(0.40f, 0.01f, 0.03f)
                )
            );
            // TT - From Oct CTP
            frontBumper.State.DiffuseColor = new Vector4(0.1f, 0.1f, 0.1f, 1.0f);

            BoxShape rearBumper = new BoxShape(
                new BoxShapeProperties("rear",
                    0.001f,
                    new Pose(new Vector3(0, 0.05f, 0.25f)),
                    new Vector3(0.40f, 0.03f, 0.03f)
                    //new Vector3(0.40f, 0.01f, 0.03f)
                )
            );
            // TT - From Oct CTP
            rearBumper.State.DiffuseColor = new Vector4(0.1f, 0.1f, 0.1f, 1.0f);

            // The physics engine will issue contact notifications only
            // if we enable them per shape
            frontBumper.State.EnableContactNotifications = true;
            rearBumper.State.EnableContactNotifications = true;

            BumperArrayEntity
                bumperArray = new BumperArrayEntity(frontBumper, rearBumper);

            // entity name, must match manifest partner name
            // TT Dec-2006 - Remove slash for V1.0
            bumperArray.State.Name = "P3DXBumpers";

            // start simulated bumper service
            // TT Dec-2006 - Subtle change for V1.0
            //CreateService(
            //    bumper.Contract.Identifier,
            //    Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
            //    "http://localhost" + bumperArray.State.Name));
            bumper.Contract.CreateService(
                ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + bumperArray.State.Name));
            return bumperArray;
        }

        // TT - Copied from Oct CTP
        private CameraEntity CreateCamera()
        {
            // low resolution, wide Field of View
            CameraEntity cam = new CameraEntity(320, 240, ((float)Math.PI * 0.4f));
            //CameraEntity cam = new CameraEntity(320, 240);
            // TT Dec-2006 - Remove slash for V1.0
            cam.State.Name = "robocam";
            // just on top of the bot
            cam.State.Pose.Position = new Vector3(0.0f, 0.5f, 0.0f);
            // reverse the orientation
            // TT Dec-2006 - No longer necessary to reverse the orientation
            // because it was only a temporary hiccup in the CTPs
            //cam.State.Pose.Orientation = Quaternion.FromAxisAngle(0, 1, 0, (float)(Math.PI));

            // camera renders in an offline buffer at each frame
            // required for service
            cam.IsRealTimeCamera = true;

            // Start simulated webcam service
            // TT Dec-2006 - Subtle change for V1.0
            //CreateService(
            //    simwebcam.Contract.Identifier,
            //    Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
            //        "http://localhost" + cam.State.Name)
            //);
            simwebcam.Contract.CreateService(
                ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + cam.State.Name)
            );

            return cam;
        }

        #endregion

        #region Lego NXT

        // TT Dec-2006 - Copied from Sim Tutorial 2
        // However, it has some strange problems because the robot
        // starts out facing the wrong direction and drives off in
        // the wrong direction initially.
        //
        // TT Jun-2007
        // Updated to use the new Differential Drive entity
        // However, it is still strange because the robot does not
        // respond to motion commands immediately, but eventually it
        // does.
        void AddLegoNxtRobot(Vector3 position)
        {
            TTLegoNXTTribot robotBaseEntity = CreateLegoNxtMotorBase(ref position);

            // Create bumper array entity and start simulated bumper service
            BumperArrayEntity bumperArray = CreateLegoNxtBumper();

            // insert as child of motor base
            robotBaseEntity.InsertEntity(bumperArray);

            // Finaly insert the motor base and its two children 
            // to the simulation
            _simEnginePort.Insert(robotBaseEntity);
        }


        private TTLegoNXTTribot CreateLegoNxtMotorBase(ref Vector3 position)
        {
            // use supplied entity that creates a motor base 
            // with 2 active wheels and one caster
            TTLegoNXTTribot robotBaseEntity = new TTLegoNXTTribot(position);

            // specify mesh. 
            robotBaseEntity.State.Assets.Mesh = "LegoNXTTribot.bos";

            // the name below must match manifest
            robotBaseEntity.State.Name = "LegoNXTMotorBase";

            // Start simulated arcos motor service
            CreateService(
                drive.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + robotBaseEntity.State.Name)
            );
            return robotBaseEntity;
        }

        private BumperArrayEntity CreateLegoNxtBumper()
        {
            // create a little bumper shape that models the NXT bumper
            BoxShape frontBumper = new BoxShape(
                new BoxShapeProperties(
                    "front", 0.001f, //mass
                    new Pose(new Vector3(0, 0.063f, -0.09f)), //position
                    new Vector3(0.023f, 0.023f, 0.045f)));

            // The physics engine will issue contact notifications only
            // if we enable them per shape
            frontBumper.State.EnableContactNotifications = true;

            BumperArrayEntity
                bumperArray = new BumperArrayEntity(frontBumper);
            // entity name, must match manifest partner name
            bumperArray.State.Name = "LegoNXTBumpers";

            // start simulated bumper service
            CreateService(
                bumper.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + bumperArray.State.Name));
            return bumperArray;
        }

        #endregion

        #endregion

        #region Handlers

        /// <summary>
        /// Get Handler
        /// </summary>
        /// <param name="get"></param>
        /// <returns></returns>
        [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
        public virtual IEnumerator<ITask> GetHandler(Get get)
        {
            get.ResponsePort.Post(_state);
            yield break;
        }

        /// <summary>
        /// Replace Handler
        /// </summary>
        /// <param name="replace"></param>
        /// <returns></returns>
        [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
        public virtual IEnumerator<ITask> ReplaceHandler(Replace replace)
        {
            _state = replace.Body;
            replace.ResponsePort.Post(DefaultReplaceResponseType.Instance);
            yield break;
        }

        #endregion

    }
}
