/* Start Header -------------------------------------------------------
Copyright (C) 20xx DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.

File Name:		Main.cpp
Purpose:		Run the game engine
Language:		C++
Platform:		g++, x86, Win32
Project:		CS529_maincpp_Final
Author:			Alex Koumandarakis, a.koumandarakis, 60001318
Creation date:	October 10, 2018
- End Header --------------------------------------------------------*/

#include "Windows.h"
#include <SDL.h>
#include "stdio.h"
#include "glew.h"
#include "GameObject.h"
#include "Input_Manager.h"
#include "FrameRateController.h"
#include "Resource_Manager.h"
#include "GameObjectManager.h"
#include "ObjectFactory.h"
#include "PhysicsManager.h"
#include "CollisionManager.h"
#include "EventManager.h"
#include "GraphicsManager.h"
#include "GameStateManager.h"

Input_Manager *gpInputManager = nullptr;
FrameRateController *gpFRC = nullptr;
ResourceManager *gpResourceManager = nullptr;
GameObjectManager *gpGameObjectManager = nullptr;
ObjectFactory *gpObjectFactory = nullptr;
PhysicsManager *gpPhysicsManager = nullptr;
CollisionManager *gpCollisionManager = nullptr;
EventManager *gpEventManager = nullptr;
GraphicsManager *gpGraphicsManager = nullptr;
GameStateManager *gpGameStateManager = nullptr;

FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
}

#pragma comment(lib, "legacy_stdio_definitions.lib")


int main(int argc, char* args[])
{
	//Allocates the console
	if (AllocConsole())
	{
		FILE* file;

		freopen_s(&file, "CONOUT$", "wt", stdout);
		freopen_s(&file, "CONOUT$", "wt", stderr);
		freopen_s(&file, "CONOUT$", "wt", stdin);

		SetConsoleTitle(L"SDL 2.0 Test");
	}

	SDL_Window *pWindow;
	int error = 0;
	bool appIsRunning = true;
	bool debug = false;

	//Initialize the managers
	gpInputManager = new Input_Manager();
	gpFRC = new FrameRateController(60);
	gpResourceManager = new ResourceManager();
	gpGameObjectManager = new GameObjectManager();
	gpObjectFactory = new ObjectFactory();
	gpPhysicsManager = new PhysicsManager();
	gpCollisionManager = new CollisionManager();
	gpEventManager = new EventManager();
	gpGameStateManager = new GameStateManager();

	//Initialize context
	SDL_GLContext openGL_context;

	// Initialize SDL
	if((error = SDL_Init( SDL_INIT_VIDEO )) < 0 )
	{
		printf("Couldn't initialize SDL, error %i\n", error);
		return 1;
	}

	//Set version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	pWindow = SDL_CreateWindow("Aster'roid-Rage",	// window title
		SDL_WINDOWPOS_UNDEFINED,					// initial x position
		SDL_WINDOWPOS_UNDEFINED,					// initial y position
		800,										// width, in pixels
		800,										// height, in pixels
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);		// Set as openGL window

	// Check that the window was successfully made
	if (NULL == pWindow)
	{
		// In the event that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	//Create opengl context
	openGL_context = SDL_GL_CreateContext(pWindow);

	//initialize glew
	if (glewInit() != GLEW_OK)
		printf("Couldn't initialize glew\n");

	if (!GLEW_VERSION_2_0)
		printf("OpenGL 2.0 is not supported\n");

	//Load Graphics Manager and shaders
	gpGraphicsManager = new GraphicsManager(pWindow);
	gpGraphicsManager->LoadShaders();
	gpGraphicsManager->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Load Main Menu
	gpObjectFactory->LoadLevel("mainMenu.txt");

	// Game loop
	while(true == appIsRunning)
	{
		gpFRC->FrameStart();
		
		if (debug) //Print frame rate info
		{
			printf("Frame Time: %f seconds\n", (float)gpFRC->GetFrameTime() / 1000.0f);
			printf("Frame Rate: %u FPS \n", 1000 / gpFRC->GetFrameTime());
		}

		SDL_Event e;
		while( SDL_PollEvent( &e ) != 0 )
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				appIsRunning = false;
			}
		}

		//Create any objects needed
		gpObjectFactory->Update();

		//Get keyboard state
		gpInputManager->Update();

		//Updating all game objects' bodies
		gpPhysicsManager->Update();
		if (debug) //Print collision info
			printf("Number of Contacts: %u\n\n", gpCollisionManager->mContacts.size());

		//Updating Event Manager
		gpEventManager->Update((float)gpFRC->GetFrameTime());

		//Updating all game objects
		for (auto go : gpGameObjectManager->mGameObjects)
		{
			go->Update();
		}

		//Draw all game object meshes
		gpGraphicsManager->DrawAll(debug);

		//Check for switching states
		gpGameStateManager->Update();

		//Check for deleted objects
		gpGameObjectManager->Update();

		//If escape is hit, quit
		if (gpInputManager->IsTriggered(SDL_SCANCODE_ESCAPE))
		{
			appIsRunning = false;
		}
		//Switch debug mode on/off
		if (gpInputManager->IsTriggered(SDL_SCANCODE_L))
		{
			debug = !debug;
		}

		gpFRC->FrameEnd();
	}

	//Free the managers
	delete(gpInputManager);
	delete(gpFRC);
	delete(gpResourceManager);
	delete(gpGameObjectManager);
	delete(gpObjectFactory);
	delete(gpPhysicsManager);
	delete(gpCollisionManager);
	delete(gpEventManager);
	delete(gpGraphicsManager);
	delete(gpGameStateManager);

	//Delete the context
	SDL_GL_DeleteContext(openGL_context);

	// Quit SDL subsystems
	SDL_Quit();
	
	return 0;
}