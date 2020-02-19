#include "../../Common/Window.h"
#include "TestEnviroment.h"

using namespace NCL;
using namespace CSC8503;

int main() {
	Window*w = Window::CreateGameWindow("Alpha Test Window", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	
	
	w->ShowOSPointer(true);
	w->LockMouseToWindow(false);

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