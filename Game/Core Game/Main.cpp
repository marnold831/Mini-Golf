#include "../../Common/Window.h"
#include "TestEnviroment.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"
#include "PxPhysicsAPI.h"
#include "PxPhysXConfig.h"
#include "foundation/PxMemory.h"
#include "extensions/PxDefaultStreams.h"
#include "extensions/PxDefaultErrorCallback.h"

using namespace NCL;
using namespace CSC8503;
using namespace physx;

int main() {
	static PxDefaultErrorCallback gDefaultErrorCallback;
	static PxDefaultAllocator gDefaultAllocatorCallback;

	auto mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
		gDefaultErrorCallback);
	if (!mFoundation)
		return -1;
	Window*w = Window::CreateGameWindow("Alpha Test Window", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	
	
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TestEnviroment* g = new TestEnviroment();
	
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();

		if (dt > 1.0f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
		/*if (!g->UpdateGame(dt)) {
			return (0);
		}*/
	}
	
	Window::DestroyGameWindow();
}