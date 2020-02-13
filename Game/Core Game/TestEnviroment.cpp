#include "TestEnviroment.h"

#include "../Game Engine Components/Layers.h"

#include "../../Common/Assets.h"
#include "../../Common/TextureLoader.h"

using namespace NCL::CSC8503;

TestEnviroment::TestEnviroment() : hasGravity(false), isDebugActive(false) {
	world		= new GameWorld();
	physics		= new PhysicsSystem(*world);
	renderer	= new GameTechRenderer(*world);

	InitLevel();
}

TestEnviroment::~TestEnviroment() {
	delete cubeMesh;
	delete basicTexture;
	delete basicShader;
	delete world;
	delete physics;
	delete renderer;
}

void TestEnviroment::UpdateGame(float dt) {
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

	// Add floor to world
	Vector3 floorPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 floorSize = Vector3(100.0f, 2.0f, 100.0f);
	Vector4 floorColour = Vector4(0.14f, 0.55f, 0.14f, 1.0f);
	world->AddGameObject(AddObject("floor", DISABLE_COLLISION_RES_LAYER, false, 0.0f, floorPosition, floorSize, floorColour));
}

void TestEnviroment::UpdateKeybinds() {
	// To be built
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
