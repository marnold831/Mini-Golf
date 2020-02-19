#include "TestEnviroment.h"

#include "../Game Engine Components/Layers.h"

#include "../../Common/Assets.h"
#include "../../Common/TextureLoader.h"

using namespace NCL::CSC8503;

TestEnviroment::TestEnviroment() : hasGravity(true), isDebugActive(false) {
	world		= new GameWorld();
	physics		= new PhysicsSystem(*world);
	renderer	= new GameTechRenderer(*world);

	InitLevel();
}

TestEnviroment::~TestEnviroment() {
	delete cubeMesh;
	delete sphereMesh;
	delete flagMesh;

	delete basicTexture;
	delete basicShader;
	delete world;
	delete physics;
	delete renderer;
}

void TestEnviroment::UpdateGame(float dt) {

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		LockedCameraMovement();
	}

	UpdateKeybinds();

	SelectObject();
	MoveSelectedObject();

	world->GetMainCamera()->UpdateCamera(dt);
	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();
}

void TestEnviroment::InitAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("goal_flag.msh", &flagMesh);

	basicTexture	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader		= new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}

void TestEnviroment::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(0.0f);
	world->GetMainCamera()->SetYaw(0.0f);
	world->GetMainCamera()->SetPosition(Vector3(0.0f, 23.5f, 20.0f));
}

void TestEnviroment::InitLevel() {
	InitAssets();
	InitCamera();

	physics->UseGravity(hasGravity);

	// Add floor to world
	Vector3 floorPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 floorSize = Vector3(100.0f, 2.0f, 100.0f);
	Vector4 floorColour = Vector4(0.14f, 0.55f, 0.14f, 1.0f);
	world->AddGameObject(AddObject("floor", CHARACTER_LAYER, false, 0.0f, floorPosition, floorSize, floorColour));

	AddGoalToWorld(Vector3(10.0f, 13.0f, 10.0f));

	world->AddGameObject(AddObject("ball", CHARACTER_LAYER, true, 5.0f, Vector3(-10.0f, 13.0f, -10.0f), Vector3(1.0f, 1.0f, 1.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)));
}

void TestEnviroment::UpdateKeybinds() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitLevel(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		hasGravity = !hasGravity; //Toggle gravity!
		physics->UseGravity(hasGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	//else {
	//	DebugObjectMovement();
	//}
}

void TestEnviroment::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);

	Vector3 upAxis = Vector3::Cross(Vector3(0, 0, 1), rightAxis);

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		selectionObject->GetPhysicsObject()->AddForce(-rightAxis * 4.0);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		selectionObject->GetPhysicsObject()->AddForce(rightAxis * 4.0);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis * 4.0);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis * 4.0);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::J)) {
		selectionObject->GetPhysicsObject()->AddForce(upAxis * 180.0);
	}


	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::LEFT)) {
		selectionObject->GetPhysicsObject()->AddTorque(-upAxis * 12.0);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::RIGHT)) {
		selectionObject->GetPhysicsObject()->AddTorque(upAxis * 12.0);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddTorque(-upAxis * 12.0);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddTorque(upAxis * 12.0);
	}
}

void  TestEnviroment::LockedCameraMovement() {
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetWorldPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);
	}
}

bool TestEnviroment::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		//renderer->DrawString("Press Q to change to camera mode!", Vector2(10, 0));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				//selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				//selectionObject->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		//renderer->DrawString("Press Q to change to select mode!", Vector2(10, 0));
	}
	return false;
}

void TestEnviroment::MoveSelectedObject() {
	//renderer->DrawString("Click on the goose and press L to start!", Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 120.0f;

	if (!selectionObject) {
		return;
	}

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

GameObject* TestEnviroment::AddObject(const string objectName, const uint32_t objectLayer, const bool isSphere,
	const float inverseMass, const Vector3 position, const Vector3 size, const Vector4 colour) {
	GameObject* object = new GameObject(objectName, objectLayer);
	CollisionVolume* volume;

	if (isSphere) {
		SphereVolume* volumeSphere = new SphereVolume(size.x);
		volume = (CollisionVolume*)volumeSphere;
	}
	else {
		AABBVolume* volumeAABB = new AABBVolume(size);
		volume = (CollisionVolume*)volumeAABB;
	}

	object->SetBoundingVolume(volume);
	object->GetTransform().SetWorldScale(size);
	object->GetTransform().SetWorldPosition(position);	
	object->SetRenderObject(new RenderObject(&object->GetTransform(), (isSphere) ? sphereMesh : cubeMesh, basicTexture, basicShader));
	object->GetRenderObject()->SetColour(colour);
	object->SetPhysicsObject(new PhysicsObject(&object->GetTransform(), object->GetBoundingVolume()));
	object->GetPhysicsObject()->SetInverseMass(inverseMass);
	object->GetPhysicsObject()->InitCubeInertia();

	return object;
}

GameObject* TestEnviroment::AddGoalToWorld(const Vector3& position) {
	GameObject* goal = new GameObject("goal");

	AABBVolume* volume = new AABBVolume(Vector3(0.4f, 10.0f, 0.4f));
	goal->SetBoundingVolume((CollisionVolume*)volume);

	goal->GetTransform().SetWorldScale(Vector3(0.4f, 0.4f, 10.0f));
	goal->GetTransform().SetWorldPosition(position);

	goal->GetTransform().SetLocalOrientation(Quaternion(Vector3(-1.0, -0.5, -0.5), 1.0));

	goal->SetRenderObject(new RenderObject(&goal->GetTransform(), flagMesh, nullptr, basicShader));
	goal->SetPhysicsObject(new PhysicsObject(&goal->GetTransform(), goal->GetBoundingVolume()));

	goal->GetPhysicsObject()->SetInverseMass(4.0f);
	goal->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(goal);

	return goal;
}