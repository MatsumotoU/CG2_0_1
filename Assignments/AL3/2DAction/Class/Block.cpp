#include "Block.h"

void Block::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore);
	model_.LoadModel("Resources", "cube.obj", COORDINATESYSTEM_HAND_RIGHT);
	model_.material_.materialData_->enableLighting = false;
}

void Block::Update() {
	model_.transform_ = transform_;
	model_.Update();
}

void Block::Draw(Camera* camera) {
	model_.Draw(camera);
}
