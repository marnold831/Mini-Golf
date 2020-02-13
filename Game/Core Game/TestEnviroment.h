/*
Author: Ciaran Halliburton & Michael Arnold
Last Edited: 13-02-2020
Descritpion: This is a basic test enviroment for developing features for the game. 
*/
#pragma once
#include "GameTechRenderer.h"

#include <string>

#include "../Game Engine Components/PhysicsSystem.h"
#include "../Game Engine Components/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"

namespace NCL {
	namespace CSC8503 {
		class TestEnviroment {
		public:
			TestEnviroment();
			~TestEnviroment();

			virtual void UpdateGame(float dt);

		protected:
			// Initialise Functions
			void InitAssets();
			void InitCamera();
			void InitLevel();
			
			// Control Functions
			void UpdateKeybinds();

			// Add Object Functions
			GameObject* AddObject(const string objectName, const uint32_t objectLayer, const bool isSphere,
				const float inverseMass, const Vector3 position, const Vector3 size, const Vector4 colour);

			GameWorld*			world;
			PhysicsSystem*		physics;
			GameTechRenderer*	renderer;

			bool hasGravity;

			bool isDebugActive;

			OGLMesh*	cubeMesh		= nullptr;
			OGLMesh*	sphereMesh		= nullptr;
			OGLTexture* basicTexture	= nullptr;
			OGLShader*	basicShader		= nullptr;
		};
	}
}