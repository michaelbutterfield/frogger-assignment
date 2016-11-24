// Frogger Assignment.cpp: A program using the TL-Engine

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <math.h>		// Include C++ Math database
using namespace tle;

//Collision function-- At bottom
float Collision(float getX, float getZ);

const float kPi = 3.1415926f;

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine(kTLX);
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder(".\\Media");

	/**** Set up your scene here ****/
	//--Camera--//
	ICamera* myCamera;
	float cameraCoord[3] = { 0.0f, 40.0f, -60.0f };
	myCamera = myEngine->CreateCamera(kManual, cameraCoord[0], cameraCoord[1], cameraCoord[2]);
	myCamera->RotateX(20);
	//--Skybox--//
	IMesh* skyboxMesh = myEngine->LoadMesh("skybox.x");				//Skybox determind by spec
	IModel* skybox = skyboxMesh->CreateModel(0, -1000, 0);
	//--Surface--//
	IMesh* surfaceMesh = myEngine->LoadMesh("surface.x");			//Water surface used for "poisoned water"
	IModel* surface = surfaceMesh->CreateModel(0, -5, 0);
	//--First Island--//
	IMesh* island1Mesh = myEngine->LoadMesh("island1.x");			//First island frogs spawn on with cars
	IModel* island1 = island1Mesh->CreateModel(0, -5, 40);
	//--Second Island--//
	IMesh* island2Mesh = myEngine->LoadMesh("island2.x");			//Island frogs have to cross "poisoned water" to get to and transition to "safe"
	IModel* island2 = island2Mesh->CreateModel(0, -5, 115);
	//--Van--//
	IMesh* transitMesh = myEngine->LoadMesh("transit.x");			//Van mesh used for model myVehicles[0], myVehicles[2] & myVehicles[4]
																	//--Car--//
	IMesh* saloonMesh = myEngine->LoadMesh("rover.x");				//Car mesh used for model myVehicles[1], myVehicles[3] & myVehicles[5]
																	//--Dummy--//
	IMesh* dummyMesh = myEngine->LoadMesh("dummy.x");				//Dummy model used to attach camera - used to move with frogs and tilt up and down
	IModel* dummy = dummyMesh->CreateModel(0, 0, 15);
	//--Tyres--//
	IMesh* tyreMesh = myEngine->LoadMesh("tyre.x");					//Tyres used for frog to cross "poisoned water"
																	//--Trees--//
	IMesh* treeMesh = myEngine->LoadMesh("plant.x");

	//Text
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	IFont* myFont = myEngine->LoadFont("Monotype Corsiva", 36);		// Load Monotype Corsiva in 36pts

	//Key Codes
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	const EKeyCode ForwardKey = (EKeyCode(192));					//User keys
	const EKeyCode BackwardKey = (EKeyCode(191));
	const EKeyCode LeftKey = Key_Z;
	const EKeyCode RightKey = Key_X;
	const EKeyCode PauseKey = Key_P;								//Transitions user between pause and playing "state"
	const EKeyCode EscapeKey = Key_Escape;							//Exits game in any game "state"
	const EKeyCode CameraUpKey = Key_Up;
	const EKeyCode CameraDownKey = Key_Down;

	//Boundaries of island1 - island2
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	const float topLimit = 114.9f;									//"Safezone" for the frogs- triggering the safe state

	const float island1minX = -55.0f;								//The mins and max's used to calculate whether the frog
	const float island1maxX = 55.0f;								//has fallen off the first island or not- triggering
	const float island1minZ = 10.0f;								//the "dead" state
	const float island1maxZ = 70.0f;

	const float island2minX = -55.0f;								//collision detection to check whether the frog has
	const float island2maxX = 55.0f;								//fallen off the tyres or has got onto the island
	const float island2minZ = 105.0f;
	const float island2maxZ = 115.0f;

	const float vehicleleftLimit = -61.9f;							//Boundary for the vehicles to "vanish"
	const float vehiclerightLimit = 61.9f;							//Boundary for the vehicles to "vanish"


	//Speeds
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	float frameTime = myEngine->Timer();							//Uses frame time of processing speed rather than 

	const float kGameSpeed = 0.04f * frameTime;						//All movement in game based off this variable
	float null = 0.0f;

	const float sinking = frameTime * 0.01;							//Speed at which the vehicles "vanish"
	const float sinkingLimit = -12.5f;								//Point at which the vehicles will switch states


	const float bounceBack = kGameSpeed;							//Small float to stop movement of camera when reaching top and bottom boundaries
	int tyreSpeed;													//Tyre movement speed

	//Text
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	const int textHoriz = 580;										//Integer used to set the horizontal co-ordinates of the win/lose text displayed
	const int textVert = 240;										//Integer used to set the vertical co-ordinates of the win/lose text displayed

	//FROGS
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	IMesh* frogMesh = myEngine->LoadMesh("frog.x");
	IMesh* frogtwoMesh = myEngine->LoadMesh("frog.x");
	IMesh* frogthreeMesh = myEngine->LoadMesh("frog.x");

	const float frogSpeed = 0.01f * frameTime;						//Setting out speed for the frogs

	const int kFrogs = 3;											//Constant int of how many frogs there are -- MAY NEED CHANGING WHEN USER CAN PLAY AGAIN WITH FROGS THAT AREN'T DEAD
	IModel* myFrogs[kFrogs];										//Creates array of models
	myFrogs[0] = frogMesh->CreateModel(-10, 0, 15);					//Placing each of the frog models within
	myFrogs[1] = frogMesh->CreateModel(0, 0, 15);					//the array and designating them their
	myFrogs[2] = frogMesh->CreateModel(10, 0, 15);					//assigned locations according to brief

	float position[kFrogs] = { -10,0,115 };
	int currentFrog = 0;											//Keeps track of which frog the player is supposed to be controlling
	const int lastFrog = 2;
	const float sitaboveRoad = 5;

	bool allSafe = false;											//Bool to track whether all frogs are safe
	bool allDead = false;											//Bool to track whether all frogs are dead
	int safeFrogs = 0;												//int to count safe frogs and change state of allSafe
	int deadFrogs = 0;												//int to count dead frogs and change state of allDead

	float frogSquash = 0.25f;										//Variable used to keep float to squash frog when hit

	float startCrossing = 15.0f;
	const float frogRadius = 4.0f;									//Frogs radius used within collision detection

	const int tempLength = 5;										//Identify the four states the frogs can be in
	const int stateLength = 4;
	enum EFrogState { waiting, crossing, safe, dead };
	EFrogState state[tempLength] = { waiting, crossing, safe, dead };

	//TYRES
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	const int kTyres = 4;											//const int for tyre array length
	const float tyreRadius = 5.0f;									//tyreRadius used for collision detection between frogs and tyres

	const int firstTyre = 0;										//constant integers to eliminate magic numbers and
	const int secondTyre = 1;										//make the code more readable
	const int thirdTyre = 2;
	const int fourthTyre = 3;
	const int tyreScale = 10;

	IModel* tyres[kTyres];											//declaration and initialisation or tyre array
	tyres[firstTyre] = tyreMesh->CreateModel(0.0f, -2.5f, 75.0f);	//initialisation and placement of each tyre in the array
	tyres[firstTyre]->Scale(tyreScale);								//scaling the tyres up by a constant int of 10 (as according to spec)
	tyres[secondTyre] = tyreMesh->CreateModel(0.0f, -2.5f, 85.0f);
	tyres[secondTyre]->Scale(tyreScale);
	tyres[thirdTyre] = tyreMesh->CreateModel(0.0f, -2.5f, 95.0f);
	tyres[thirdTyre]->Scale(tyreScale);
	tyres[fourthTyre] = tyreMesh->CreateModel(0.0f, -2.5f, 105.0f);
	tyres[fourthTyre]->Scale(tyreScale);

	//TREES
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	const int kTrees = 6;											//Use a constant for the amount of trees in the array
	const int firstTree = 0;										//const ints for each of the trees in the array to eliminate magic numbers
	const int secondTree = 1;										//const ints for each of the trees in the array to eliminate magic numbers
	const int thirdTree = 2;										//const ints for each of the trees in the array to eliminate magic numbers
	const int fourthTree = 3;										//const ints for each of the trees in the array to eliminate magic numbers
	const int fifthTree = 4;										//const ints for each of the trees in the array to eliminate magic numbers
	const int sixthTree = 5;										//const ints for each of the trees in the array to eliminate magic numbers

	IModel* trees[kTrees];											//Array of trees
	trees[firstTree] = treeMesh->CreateModel(-50.0f, 0.0f, 115.0f);	//First tree create model at position (-50.0f, 0.0f, 115.0f)
	trees[secondTree] = treeMesh->CreateModel(-30.0f, 0.0f, 115.0f);//Second tree create model at position (-30.0f, 0.0f, 115.0f)
	trees[thirdTree] = treeMesh->CreateModel(-10.0f, 0.0f, 115.0f);	//Third tree create model at position (-10.0f, 0.0f, 115.0f)
	trees[fourthTree] = treeMesh->CreateModel(10.0f, 0.0f, 115.0f);	//Fourth tree create model at position (10.0f, 0.0f, 115.0f)
	trees[fifthTree] = treeMesh->CreateModel(30.0f, 0.0f, 115.0f);	//Fifth tree create model at position (30.0f, 0.0f, 115.0f)
	trees[sixthTree] = treeMesh->CreateModel(50.0f, 0.0f, 115.0f);	//Sixth tree create model at position (50.0f, 0.0f, 115.0f)

	//VEHICLES
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	const int kVehicles = 6;										//const int to use for array
	const int kVehicleStateLength = 8;
	const float van1startingPosition[3] = { 50.0f, 0.0f, 35.0f };
	const float car1startingPosition[3] = { -50.0f, 0.0f, 45.0f };	//Arrays to hold the starting position coordinates of both the van and saloon
	const float van2startingPosition[3] = { -10.0f, 0.0f, 35.0f };
	const float car2startingPosition[3] = { 10.0f, 0.0f, 45.0f };
	const float van3startingPosition[3] = { 30.0f, 0.0f, 35.0f };
	const float car3startingPosition[3] = { 30.0f, 0.0f, 45.0f };


	float lane2Speed = frameTime * 0.01f;							// frameTime * 0.01
	float lane3Speed = lane2Speed * 1.5f;							// 1.5 * lane2Speed (as according to spec)

	float van1Speed = lane2Speed;									// frameTime * 0.01 Speeds to be able to stop and start the cars individually (e.g. when getting to the edge, instead of stopping the whole lane, just one car can stop)
	float van2Speed = lane2Speed;									// frameTime * 0.01
	float van3Speed = lane2Speed;									// frameTime * 0.01
	float car1Speed = lane3Speed;									// lane2Speed * 1.5 (frameTime * 0.01 * 1.5)
	float car2Speed = lane3Speed;									// lane2Speed * 1.5 (frameTime * 0.01 * 1.5)
	float car3Speed = lane3Speed;

	const int van1 = 0;												//Van that spawns on the right edge of island 1
	const int van2 = 2;												//Van that spawns near the centre of island 1
	const int van3 = 4;												//Van that spawns inbetween 1 & 2
	const int car1 = 1;												//Car that spawns on the left edge of island 1
	const int car2 = 3;												//Car that spawns near the centre of island 1
	const int car3 = 5;												//Car that spawns inbetween 1 & 2

	IModel* myVehicles[kVehicles];									//Array to store the vehicles in

	//CREATE MODELS
	myVehicles[van1] = transitMesh->CreateModel(van1startingPosition[0], van1startingPosition[1], van1startingPosition[2]);			//Calls the coordinates stored in the vanstartingPosition array 
	myVehicles[van1]->RotateY(270);									//Rotates it 270 degrees

	myVehicles[van2] = transitMesh->CreateModel(van2startingPosition[0], van2startingPosition[1], van2startingPosition[2]);
	myVehicles[van2]->RotateY(270);

	myVehicles[van3] = transitMesh->CreateModel(van3startingPosition[0], van3startingPosition[1], van3startingPosition[2]);
	myVehicles[van3]->RotateY(270);

	myVehicles[car1] = saloonMesh->CreateModel(car1startingPosition[0], car1startingPosition[1], car1startingPosition[2]);	//Calls the coordinates stored for the saloon in the array
	myVehicles[car1]->RotateY(90);									//Then rotates the model 90 degress - fitting with the spec

	myVehicles[car2] = saloonMesh->CreateModel(car2startingPosition[0], car2startingPosition[1], car2startingPosition[2]);
	myVehicles[car2]->RotateY(90);

	myVehicles[car3] = saloonMesh->CreateModel(car3startingPosition[0], car3startingPosition[1], car3startingPosition[2]);
	myVehicles[car3]->RotateY(90);

	const float carriseX = -61.9f;									//Used within vehicle loops- -61.9f- X coordinate the cars rise at
	const float vanriseX = 61.9f;									//Used within vehicle loops- 61.9f- X coordinate the vans rise at
	const float vehicleriseY = 0.0f;								//Used within the vehicle loops- limit to stop the vehicles rising above the road
	const float vanriseZ = 35.0f;									//Used within vehicle loops- Z coordinates at which the vans will rise
	const float carriseZ = 45.0f;									//Used within the vehicle loops- Z coordinates at which the cars will rise

	//Counters
	int counter1 = 0;												//Used to count frames when van1 vanishes (waits 2 seconds) then reappears and rises
	int counter2 = 0;												//Used to count frames when van2 vanishes (waits 2 seconds) then reappears and rises
	int counter3 = 0;												//Used to count frames when van3 vanishes (waits 2 seconds) then reappears and rises
	int counter4 = 0;												//Used to count frames when car1 vanishes (waits 2 seconds) then reappears and rises
	int counter5 = 0;												//Used to count frames when car2 vanishes (waits 2 seconds) then reappears and rises
	int counter6 = 0;												//Used to count frames when car3 vanishes (waits 2 seconds) then reappears and rises

	int wait = 2500;												//Two seconds worth of frames

	//VEHICLE COLLISION DETECTION
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
	float vehicleminX = 0;											//Used to calculate the
	float vehiclemaxX = 5;											//sphere to box collision
	float vehicleminZ = 0;											//between the frog and the vehicles
	float vehiclemaxZ = 10;

	//States
	bool paused = false;											// Game state to stop and start movement
	bool overState = false;											// Game state to end and finish the game when a set of parameters have been met

	//Dummy
	myCamera->AttachToParent(dummy);

	//Camera
	float cameraupperLimit = 60.0f;									//When controlling the camera up - stops it from going through the ceiling
	float cameralowerLimit = 5.0f;									//When controlling the camera down - stops it going through the floor and inverting

	//Array Elements
	int firstElement = 0;											//First element in the array
	int secondElement = 1;											//Second element in the array
	int thirdElement = 2;											//Third element in the array

	//Pre-load skins to quicken process times
	myFrogs[currentFrog]->SetSkin("frog_red.bmp");
	myFrogs[currentFrog]->SetSkin("frog_blue.bmp");
	myFrogs[currentFrog]->SetSkin("frog.bmp");

	//---------------------------------------------------------------------------------------------MAIN GAME--------------------------------------------------------------------------------------------------//
	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		myEngine->Timer();

		//IF NOT PAUSED - THIS WILL RUN
		if (!paused)
		{
			//Start the frogs off.
			if (myFrogs[currentFrog]->GetZ() == startCrossing)		//When the player starts the game it will select the first frog
			{														//and as all the frogs are placed on the same Z coordinate
				state[stateLength] = crossing;						//it will allow only the currently selected frog to move
			}

			//CAMERA
			//------------------------------------------
			myCamera->LookAt(dummy);
			if (myEngine->KeyHeld(CameraUpKey))						//Holding the up key on the keyboard will allow to player to tilt the camera up, centred on the frog
			{
				myCamera->MoveLocalY(kGameSpeed);
			}
			if (myEngine->KeyHeld(CameraDownKey))					//Holding the up key on the keyboard will allow to player to tilt the camera down, centred on the frog
			{
				myCamera->MoveLocalY(-kGameSpeed);
			}
			if (myCamera->GetLocalY() > cameraupperLimit)			//Enforcing an upper limit for the camera so the player cannot go out of bounds, the camera turning up side down and breaking the game
			{
				myCamera->MoveLocalY(-bounceBack);					//Moves the camera at the same speed in the opposite direction in order to give the look of no movement
			}
			if (myCamera->GetLocalY() < cameralowerLimit)			//Enforcing an lower limit for the camera so the player cannot go out of bounds, the camera turning up side down and breaking the game
			{
				myCamera->MoveLocalY(bounceBack);					//Moves the camera at the same speed in the opposite direction in order to give the look of no movement
			}

			//FROG SAFE
			if (myFrogs[currentFrog]->GetZ() > topLimit)			//If not last frog and the frogs Z coordinate is within
			{														//the endzone/safezone then change the frogs state to SAFE
				state[stateLength] = safe;
			}

			//--------------------------------------------------------------- SWITCH CASES - States for frogs ---------------------------------------------------------------//
			switch (state[stateLength])
			{
			case waiting:											//Frogs immediately go into waiting until state has been changed
			{														//allowing only one frog (currentFrog) to be moved

			}
			case crossing:
			{
				//Input keys for users
				//-------------------------------------------------
				if (myEngine->KeyHeld(ForwardKey))					//If forward key is held, gets frog currently in crossing state from array and allows to move forwards
				{
					myFrogs[currentFrog]->MoveLocalZ(frogSpeed);	//Move at 0.01 * frameTime
					dummy->MoveLocalZ(frogSpeed);
				}
				if (myEngine->KeyHeld(BackwardKey))					//If backwards key is held, gets frog currently in crossing state from array and allows to move backwards
				{
					myFrogs[currentFrog]->MoveLocalZ(-frogSpeed);
					dummy->MoveLocalZ(-frogSpeed);
				}
				if (myEngine->KeyHeld(LeftKey))						//If left key is held, gets frog currently in crossing state from array and allows to move left
				{
					myFrogs[currentFrog]->MoveX(-frogSpeed);
				}
				if (myEngine->KeyHeld(RightKey))					//If right key is held, gets frog currently in crossing state from array and allows to move right
				{
					myFrogs[currentFrog]->MoveX(frogSpeed);
				}
				break;
			}
			case safe:
			{
				safeFrogs++;										//Adds 1 to an integer to keep track of frogs in the "safe" state
				dummy->SetPosition(null, null, startCrossing);		//Before resetting for the next frog to attach and move with
				if (currentFrog == lastFrog)
				{
					myFrogs[currentFrog]->SetSkin("frog_blue.bmp");	//All frogs should be blue and safe
					overState = true;								//Sets over state to true and finishes the game
				}
				else
				{
					myFrogs[currentFrog]->SetSkin("frog_blue.bmp");
					currentFrog++;
				}
				break;
			}
			case dead:												//If frog collides with vehicle,  changes state to dead
			{
				deadFrogs++;
				myFrogs[currentFrog]->DetachFromParent();			//Detatches from dummy model
				dummy->SetPosition(null, null, startCrossing);		//Before resetting for the next frog to attach and move with
				if (currentFrog == lastFrog)						//If the last frog
				{
					myFrogs[currentFrog]->SetSkin("frog_red.bmp");	//Changes skin to already preloaded red skin
					myFrogs[currentFrog]->ScaleY(frogSquash);		//Reduces the Y scale of the model to 0.25f of its scale to look like its been run over 
					overState = true;								//Changes the over state to true
				}
				else
				{
					myFrogs[currentFrog]->SetSkin("frog_red.bmp");	//Changes skin to already preloaded red skin
					myFrogs[currentFrog]->ScaleY(frogSquash);		//Reduces the Y scale of the model to 0.25f of its scale to look like its been run over 
					currentFrog++;									//+1 to the next frog in the array
				}

			}
			} // end of switch scope

			  /**** Update your scene each frame here ****/
			  //----------------------------------------------------------------------- GAME OVER STATE -----------------------------------------------------------------------//
			  //"GAME OVER MAN" - Bill Paxton, Aliens (d. James Cameron, 1986)
			if (overState == true)
			{
				if (safeFrogs == kFrogs)							//If safeFrogs == 3 then allSafe state is true
				{
					allSafe = true;
				}
				else if (deadFrogs == kFrogs)						//If deadFrogs == 3 then allDead is true
				{
					allDead = true;
				}
				if (allSafe == true)
				{
					van1Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					van2Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					van3Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					car1Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					car2Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					car3Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					myFont->Draw("You Win!", textHoriz, textVert, kGreen);	//Displays win text in the centre of the screen
				}
				else if (allDead == true)
				{
					van1Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					van2Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					van3Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					car1Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					car2Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					car3Speed = null;								//Pausing the game causes errors with text as it has to be refreshed every frame
					myFont->Draw("FAILURE.", textHoriz, textVert, kRed);	//Displays lose text in the centre of the screen
				}
			}

			//-------------------------------------------------------------------- COLLISION DETECTION ---------------------------------------------------------------------//
			//Vehicle Collision Detection
			//-------------------------------------------
			float frogX = myFrogs[currentFrog]->GetX();				//Gets the current frogs X coordinates to - vehicles X coordinates in collision detection
			float frogZ = myFrogs[currentFrog]->GetZ();				//Gets the current frogs Z coordinates to - vehicles Z coordinates in collision detection

																	//Shortens names and simplifies to make code more legible for those who don't know what it does
			float van1X = myVehicles[van1]->GetX();					//Gets van1 X coordinates in order to update collision detection
			float van1Y = myVehicles[van1]->GetY();					//Gets van1 Y coordinates in order to update the vehicle loop
			float van1Z = myVehicles[van1]->GetZ();					//Gets van1 Z coordinates in order to update collision detection
			float van2X = myVehicles[van2]->GetX();					//Gets van2 X coordinates in order to update collision detection
			float van2Y = myVehicles[van2]->GetY();					//Gets van2 Y coordinates in order to update the vehicle loop
			float van2Z = myVehicles[van2]->GetZ();					//Gets van2 Z coordinates in order to update collision detection
			float van3X = myVehicles[van3]->GetX();					//Gets van3 X coordinates in order to update collision detection
			float van3Y = myVehicles[van3]->GetY();					//Gets van3 Y coordinates in order to update the vehicle loop					
			float van3Z = myVehicles[van3]->GetZ();					//Gets van3 Z coordinates in order to update collision detection
			float car1X = myVehicles[car1]->GetX();					//Gets car1 X coordinates in order to update collision detection
			float car1Y = myVehicles[car1]->GetY();					//Gets car1 Y coordinates in order to update the vehicle loop
			float car1Z = myVehicles[car1]->GetZ();					//Gets car1 Z coordinates in order to update collision detection
			float car2X = myVehicles[car2]->GetX();					//Gets car2 X coordinates in order to update collision detection
			float car2Y = myVehicles[car2]->GetY();					//Gets car2 Y coordinates in order to update the vehicle loop
			float car2Z = myVehicles[car2]->GetZ();					//Gets car2 Z coordinates in order to update collision detection
			float car3X = myVehicles[car3]->GetX();					//Gets car3 X coordinates in order to update collision detection
			float car3Y = myVehicles[car3]->GetY();					//Gets car3 Y coordinates in order to update the vehicle loop
			float car3Z = myVehicles[car3]->GetZ();					//Gets car3 Z coordinates in order to update collision detection

			float van1Collision = Collision(frogX - van1X, frogZ - van1Z); //Triggers function call - sends frogs x & Z coordinates and van1 x & z coordinates every second, doing the math and returning the value
			float van2Collision = Collision(frogX - van2X, frogZ - van2Z); //Triggers function call - sends frogs x & Z coordinates and van2 x & z coordinates every second, doing the math and returning the value
			float van3Collision = Collision(frogX - van3X, frogZ - van3Z); //Triggers function call - sends frogs x & Z coordinates and van3 x & z coordinates every second, doing the math and returning the value
			float car1Collision = Collision(frogX - car1X, frogZ - car1Z); //Triggers function call - sends frogs x & Z coordinates and car1 x & z coordinates every second, doing the math and returning the value
			float car2Collision = Collision(frogX - car2X, frogZ - car2Z); //Triggers function call - sends frogs x & Z coordinates and car2 x & z coordinates every second, doing the math and returning the value
			float car3Collision = Collision(frogX - car3X, frogZ - car3Z); //Triggers function call - sends frogs x & Z coordinates and car3 x & z coordinates every second, doing the math and returning the value

																		   //Van 1 Collision (Van that starts at the right most edge)
			if (van1Collision < frogRadius && van1Collision > vehicleminX && van1Collision < vehiclemaxX || van1Collision > vehicleminZ && van1Collision < vehiclemaxZ)
			{
				state[stateLength] = dead;									//If the collision distance is less than the frogs radius AND the distance is within the vehicles boundaries (determined by GetX and GetZ)- then collision occurs
			}

			//Van 2 Collision (One that starts near the centre)
			if (van2Collision < frogRadius && van2Collision > vehicleminX && van2Collision < vehiclemaxX || van2Collision > vehicleminZ && van2Collision < vehiclemaxZ)
			{
				state[stateLength] = dead;									//If the collision distance is less than the frogs radius AND the distance is within the vehicles boundaries (determined by GetX and GetZ)- then collision occurs
			}

			//Van 3 Collision (One that starts inbetween 1 & 2)
			if (van3Collision < frogRadius && van3Collision > vehicleminX && van3Collision < vehiclemaxX || van3Collision > vehicleminZ && van3Collision < vehiclemaxZ)
			{
				state[stateLength] = dead;									//If the collision distance is less than the frogs radius AND the distance is within the vehicles boundaries (determined by GetX and GetZ)- then collision occurs
			}

			//Car1 Collision
			if (car1Collision < frogRadius && car1Collision > vehicleminX && car1Collision < vehiclemaxX || car1Collision > vehicleminZ && car1Collision < vehiclemaxZ)
			{
				state[stateLength] = dead;									//If the collision distance is less than the frogs radius AND the distance is within the vehicles boundaries (determined by GetX and GetZ)- then collision occurs
			}

			//Car 2 Collision
			if (car2Collision < frogRadius && car2Collision > vehicleminX && car2Collision < vehiclemaxX || car2Collision > vehicleminZ && car2Collision < vehiclemaxZ)
			{
				state[stateLength] = dead;									//If the collision distance is less than the frogs radius AND the distance is within the vehicles boundaries (determined by GetX and GetZ)- then collision occurs
			}

			//Car 3 Collision
			if (car3Collision < frogRadius && car3Collision > vehicleminX && car3Collision < vehiclemaxX || car3Collision > vehicleminZ && car3Collision < vehiclemaxZ)
			{
				state[stateLength] = dead;									//If the collision distance is less than the frogs radius AND the distance is within the vehicles boundaries (determined by GetX and GetZ)- then collision occurs
			}

			//Island Collision Detection
			//---------------------------------------------
			//If currentFrog is OFF island 1
			if (frogX < island1minX || frogX > island1maxX || frogZ < island1minZ || frogZ > island1maxZ)
			{
				state[stateLength] = dead;
			}

			//if currentFrog is ON island 2 -- for when coming off the tyres
			if (frogX > island2minX && frogX < island2maxX && frogZ > island2minZ && frogZ < island2maxZ)
			{
				state[stateLength] = crossing;
			}

			//Tyre Collision
			float tyre1X = tyres[firstTyre]->GetX();							//Shortens names and simplifies to make code more legible for those who don't know what it does
			float tyre1Z = tyres[firstTyre]->GetZ();							//Gets X and Z of vehicles every frame in order to update collision detection
			float tyre2X = tyres[secondTyre]->GetX();
			float tyre2Z = tyres[secondTyre]->GetZ();
			float tyre3X = tyres[thirdTyre]->GetX();
			float tyre3Z = tyres[thirdTyre]->GetZ();
			float tyre4X = tyres[fourthTyre]->GetX();
			float tyre4Z = tyres[fourthTyre]->GetZ();

			float tyre1Collision = Collision(frogX - tyre1X, frogZ - tyre1Z);	//Uses the vectors between the frog and each
			float tyre2Collision = Collision(frogX - tyre2X, frogZ - tyre2Z);	//of the tyres, goes into the function and calculates
			float tyre3Collision = Collision(frogX - tyre3X, frogZ - tyre3Z);	//the distance of the vector
			float tyre4Collision = Collision(frogX - tyre4X, frogZ - tyre4Z);

			//First Tyre
			if (tyre1Collision < frogRadius + tyreRadius)						//If the distance is lower than the radius of the frog and tyre combined-there is a collision, which means the frog is floating on the tyre	
			{
				//myFrogs[currentFrog]->AttachToParent(tyres[firstTyre]);		//Attaches the frog to the tyre to parent the tyres movements to the frog and simulate the "floating across a river" effect
				state[stateLength] = crossing;
			}
			/*	else if (tyre1Collision > frogRadius + tyreRadius)
			{
			myFrogs[currentFrog]->DetachFromParent();
			}*/

			//Second Tyre
			if (tyre2Collision < frogRadius + tyreRadius)						//If the distance is lower than the radius of the frog and tyre combined-there is a collision, which means the frog is floating on the tyre	
			{
				//myFrogs[currentFrog]->AttachToParent(tyres[secondTyre]);		//Attaches the frog to the tyre to parent the tyres movements to the frog and simulate the "floating across a river" effect
				state[stateLength] = crossing;
			}
			/*else if (tyre2Collision > frogRadius + tyreRadius)
			{
			myFrogs[currentFrog]->DetachFromParent();
			}*/

			//Third Tyre
			if (tyre3Collision < frogRadius + tyreRadius)						//If the distance is lower than the radius of the frog and tyre combined-there is a collision, which means the frog is floating on the tyre	
			{
				//myFrogs[currentFrog]->AttachToParent(tyres[thirdTyre]);		//Attaches the frog to the tyre to parent the tyres movements to the frog and simulate the "floating across a river" effect
				state[stateLength] = crossing;
			}
			/*else if (tyre3Collision > frogRadius + tyreRadius)
			{
			myFrogs[currentFrog]->DetachFromParent();
			}*/

			//Fourth Tyre
			if (tyre4Collision < frogRadius + tyreRadius)						//If the distance is lower than the radius of the frog and tyre combined-there is a collision, which means the frog is floating on the tyre	
			{
				//myFrogs[currentFrog]->AttachToParent(tyres[fourthTyre]);		//Attaches the frog to the tyre to parent the tyres movements to the frog and simulate the "floating across a river" effect
				state[stateLength] = crossing;
			}
			/*else if (tyre4Collision > frogRadius + tyreRadius)
			{
			myFrogs[currentFrog]->DetachFromParent();
			}*/

			//TREE COLLISION
			float tree1X = trees[firstTree]->GetX();									//Trees on the far side of the island that the frog can collide with
			float tree1Z = trees[firstTree]->GetZ();									//Getting X and Z's to do sphere to square collision
			float tree2X = trees[secondTree]->GetX();
			float tree2Z = trees[secondTree]->GetZ();
			float tree3X = trees[thirdTree]->GetX();
			float tree3Z = trees[thirdTree]->GetZ();
			float tree4X = trees[fourthTree]->GetX();
			float tree4Z = trees[fourthTree]->GetZ();
			float tree5X = trees[fifthTree]->GetX();
			float tree5Z = trees[fifthTree]->GetZ();
			float tree6X = trees[sixthTree]->GetX();
			float tree6Z = trees[sixthTree]->GetZ();

			float tree1Collision = Collision(frogX - tree1X, frogZ - tree1Z);			//Taking two vectors (frogX, FrogZ & treeX, treeZ) and calculating the length of the vector, or distance
			float tree2Collision = Collision(frogX - tree2X, frogZ - tree2Z);			//Taking two vectors (frogX, FrogZ & treeX, treeZ) and calculating the length of the vector, or distance
			float tree3Collision = Collision(frogX - tree3X, frogZ - tree3Z);			//Taking two vectors (frogX, FrogZ & treeX, treeZ) and calculating the length of the vector, or distance
			float tree4Collision = Collision(frogX - tree4X, frogZ - tree4Z);			//Taking two vectors (frogX, FrogZ & treeX, treeZ) and calculating the length of the vector, or distance
			float tree5Collision = Collision(frogX - tree5X, frogZ - tree5Z);			//Taking two vectors (frogX, FrogZ & treeX, treeZ) and calculating the length of the vector, or distance
			float tree6Collision = Collision(frogX - tree6X, frogZ - tree6Z);			//Taking two vectors (frogX, FrogZ & treeX, treeZ) and calculating the length of the vector, or distance

																						//TREE1
			if (tree1Collision < frogRadius)											//If the frog collides with one of the trees, set state to dead
			{
				state[stateLength] = dead;
			}

			//TREE2
			if (tree2Collision < frogRadius)											//If the frog collides with one of the trees, set state to dead
			{
				state[stateLength] = dead;
			}

			//TREE3
			if (tree3Collision < frogRadius)											//If the frog collides with one of the trees, set state to dead
			{
				state[stateLength] = dead;
			}

			//TREE4
			if (tree4Collision < frogRadius)											//If the frog collides with one of the trees, set state to dead
			{
				state[stateLength] = dead;
			}

			//TREE5
			if (tree5Collision < frogRadius)											//If the frog collides with one of the trees, set state to dead
			{
				state[stateLength] = dead;
			}

			//TREE6
			if (tree6Collision < frogRadius)											//If the frog collides with one of the trees, set state to dead
			{
				state[stateLength] = dead;
			}

			//VEHICLE CODE
			//--------------------------------------------------------
			//Van 1 Loop
			//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
			if (van1X <= vanriseX && van1Y >= vehicleriseY)			//Starts the van loop off, setting the vehicle off
			{
				myVehicles[van1]->MoveLocalZ(van1Speed);
			}

			if (van1X <= carriseX && van1Y >= sinkingLimit)			//If above the floor limit and off the right edge of the island then sink into floor
			{
				myVehicles[van1]->MoveY(-sinking);
			}

			if (van1Y <= sinkingLimit)								//When below the floor...
			{
				counter1++;
				if (counter1 == wait)								//Wait ~2 seconds
				{
					myVehicles[van1]->SetPosition(vanriseX, sinkingLimit, vanriseZ);//Set the position of the van below the floor off the left edge of the island
					counter1 = null;								//Reset counter
				}
			}

			if (van1Y <= vehicleriseY && van1X >= vanriseX)			//If below 0 and more than the right edge raise the van
			{
				myVehicles[van1]->MoveLocalY(sinking);
			}														//Code loops~~~

																	//Van 2 Loop
																	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

			if (van2X <= vanriseX && van2Y >= vehicleriseY)			//Starts the van loop off, setting the vehicle off
			{
				myVehicles[van2]->MoveLocalZ(van2Speed);
			}

			if (van2X <= carriseX && van2Y >= sinkingLimit)			//If above the floor limit and off the right edge of the island then sink into floor
			{
				myVehicles[van2]->MoveY(-sinking);
			}

			if (van2Y <= sinkingLimit)								//When below the floor...
			{
				counter2++;
				if (counter2 == wait)								//Wait ~2 seconds
				{
					myVehicles[van2]->SetPosition(vanriseX, sinkingLimit, vanriseZ);//Set the position of the van below the floor off the left edge of the island
					counter2 = null;								//Reset counter
				}
			}

			if (van2Y <= vehicleriseY && van2X >= vanriseX)			//If below 0 and more than the right edge raise the van
			{
				myVehicles[van2]->MoveLocalY(sinking);
			}

			//Van 3 Loop
			//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

			if (van3X <= vanriseX && van3Y >= vehicleriseY)			//Starts the van loop off, setting the vehicle off
			{
				myVehicles[van3]->MoveLocalZ(van3Speed);
			}

			if (van3X <= carriseX && van3Y >= sinkingLimit)			//If above the floor limit and off the right edge of the island then sink into floor
			{
				myVehicles[van3]->MoveY(-sinking);
			}

			if (van3Y <= sinkingLimit)								//When below the floor...
			{
				counter5++;
				if (counter5 == wait)								//Wait ~2 seconds
				{
					myVehicles[van3]->SetPosition(vanriseX, sinkingLimit, vanriseZ);//Set the position of the van below the floor off the left edge of the island
					counter5 = null;								//Reset counter
				}
			}

			if (van3Y <= vehicleriseY && van3X >= vanriseX)			//If below 0 and more than the right edge raise the van
			{
				myVehicles[van3]->MoveLocalY(sinking);
			}

			//Car 1 Loop
			//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

			if (car1X >= carriseX && car1Y >= vehicleriseY)			//Starts the van loop off, setting the vehicle off
			{
				myVehicles[car1]->MoveLocalZ(car1Speed);
			}

			if (car1X >= vanriseX && car1Y >= sinkingLimit)			//If above the floor limit and off the right edge of the island then sink into floor
			{
				myVehicles[car1]->MoveY(-sinking);
			}

			if (car1Y <= sinkingLimit)								//When below the floor...
			{
				counter3++;
				if (counter3 == wait)								//Wait ~2 seconds
				{
					myVehicles[car1]->SetPosition(carriseX, sinkingLimit, carriseZ);//Set the position of the van below the floor off the left edge of the island
					counter3 = null;								//Reset counter
				}
			}

			if (car1Y <= vehicleriseY && car1X <= carriseX)			//If below 0 and more than the right edge raise the van
			{
				myVehicles[car1]->MoveLocalY(sinking);
			}

			//Car 2 Loop
			//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

			if (car2X >= carriseX && car2Y >= vehicleriseY)			//Starts the van loop off, setting the vehicle off
			{
				myVehicles[car2]->MoveLocalZ(car2Speed);
			}

			if (car2X >= vanriseX && car2Y >= sinkingLimit)			//If above the floor limit and off the right edge of the island then sink into floor
			{
				myVehicles[car2]->MoveY(-sinking);
			}

			if (car2Y <= sinkingLimit)								//When below the floor...
			{
				counter4++;
				if (counter4 == wait)								//Wait ~2 seconds
				{
					myVehicles[car2]->SetPosition(carriseX, sinkingLimit, carriseZ);//Set the position of the van below the floor off the left edge of the island
					counter4 = null;								//Reset counter
				}
			}

			if (car2Y <= vehicleriseY && car2X <= carriseX)			//If below 0 and more than the right edge raise the van
			{
				myVehicles[car2]->MoveLocalY(sinking);
			}

			//Car 3 Loop
			//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

			if (car3X >= carriseX && car3Y >= vehicleriseY)			//Starts the van loop off, setting the vehicle off
			{
				myVehicles[car3]->MoveLocalZ(car3Speed);
			}

			if (car3X >= vanriseX && car3Y >= sinkingLimit)			//If above the floor limit and off the right edge of the island then sink into floor
			{
				myVehicles[car3]->MoveY(-sinking);
			}

			if (car3Y <= sinkingLimit)								//When below the floor...
			{
				counter6++;
				if (counter6 == wait)								//Wait ~2 seconds
				{
					myVehicles[car3]->SetPosition(carriseX, sinkingLimit, carriseZ);//Set the position of the van below the floor off the left edge of the island
					counter6 = null;								//Reset counter
				}
			}

			if (car3Y <= vehicleriseY && car3X <= carriseX)			//If below 0 and more than the right edge raise the van
			{
				myVehicles[car3]->MoveLocalY(sinking);
			}

		} // end if not paused

		  //Will happen whether paused or not
		if (myEngine->KeyHit(PauseKey))
		{
			paused = !paused;					//Reverse bools state (!pause becomes pause, pause becomes !pause)
		}

		if (myEngine->KeyHit(Key_Escape))
		{
			myEngine->Stop();					//Exit game from any state
		}
	}


	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

float Collision(float getX, float getZ)			//Function used to do calculations for all collision detections
{
	float collision = sqrt(getX*getX + getZ*getZ);
	return collision;
}