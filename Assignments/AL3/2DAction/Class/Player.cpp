#include "Player.h"
#include <algorithm>
#include "../../../../Engine/Math/MyMath.h"
#include "MapChip.h"

void Player::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	turnTime_ = 0.0f;
	turnTable[0] = 3.14f / 2.0f + 3.14f;
	turnTable[1] = 3.14f / 2.0f;
	lrDirection_ = LRDirection::kRight;

	transform_.scale.x = 2.0f;
	transform_.scale.y = 2.0f;
	transform_.scale.z = 2.0f;
	transform_.rotate.y = 3.14f / 2.0f;

	velocity_ = {};
	acceleration_ = {};

	isGround_ = true;
	isLanding_ = false;

	map_ = nullptr;
	isActive_ = true;
}

void Player::Update() {
	if (!isActive_) {
		return;
	}

	// マップの位置取得
	uint32_t xIndex = static_cast<uint32_t>(transform_.translate.x / kBlockWidth);
	uint32_t yIndex = kNumBlockVertical - static_cast<uint32_t>(transform_.translate.y / kBlockHeight) - 1;
	xIndex;
	yIndex;

	/*if (!isGround_) {
		if (transform_.translate.y < 3.0f) {
			isGround_ = true;
			isLanding_ = false;
			acceleration_.y = 0.0f;
			velocity_.y = 0.0f;
			transform_.translate.y = 3.0f;
		}
	}*/

	if (map_->GetMapChipTypeByIndex(xIndex, yIndex + 1) == MapChipType::kBlank) {
		isGround_ = false;
		isLanding_ = true;
	}

	if (!isGround_) {
		if (map_->GetMapChipTypeByIndex(xIndex, yIndex + 1) == MapChipType::kBlock) {
			if ((map_->GetMapChipPositionByIndex(xIndex, yIndex + 1) - transform_.translate).Length() <= kBlockWidth) {
				isGround_ = true;
				isLanding_ = false;
				acceleration_.y = 0.0f;
				velocity_.y = 0.0f;

				transform_.translate.y = map_->GetMapChipPositionByIndex(xIndex, yIndex + 1).y + kBlockHeight;
			}
		}
	}

	DirectInputManager* input = engineCore_->GetInputManager();
	if (input->keyboard_.GetPress(DIK_RIGHT)) {
		acceleration_.x += kAcceleration * engineCore_->GetDeltaTime();

		if (lrDirection_ != LRDirection::kRight) {
			turnTime_ = kTimeTurn;
			nowTurnY_ = transform_.rotate.y;
			lrDirection_ = LRDirection::kRight;
		}
	}
	if (input->keyboard_.GetPress(DIK_LEFT)) {
		acceleration_.x -= kAcceleration * engineCore_->GetDeltaTime();

		if (lrDirection_ != LRDirection::kLeft) {
			turnTime_ = kTimeTurn;
			nowTurnY_ = transform_.rotate.y;
			lrDirection_ = LRDirection::kLeft;
		}
	}
	if (input->keyboard_.GetPress(DIK_UP)) {
		if (!isLanding_) {
			isLanding_ = true;
			isGround_ = false;
			velocity_.y += kJumpPower;
		}
	}

	if (!isGround_) {
		acceleration_.y -= 9.8f * engineCore_->GetDeltaTime();
	}
	
	acceleration_.y *= 0.9f;
	acceleration_.x *= 0.9f;
	
	acceleration_.x = std::clamp(acceleration_.x, -kMaxAcceleration, kMaxAcceleration);

	velocity_.x += acceleration_.x;
	velocity_.y += acceleration_.y;

	velocity_.x *= 0.98f;
	velocity_.y *= 0.98f;

	velocity_.x = std::clamp(velocity_.x, -kMaxVelocity, kMaxVelocity);

	transform_.translate.x += velocity_.x * engineCore_->GetDeltaTime();
	transform_.translate.y += velocity_.y * engineCore_->GetDeltaTime();

	// 回転処理
	if (turnTime_ > 0.0f) {
		turnTime_ -= engineCore_->GetDeltaTime();
		float t = turnTime_ / kTimeTurn;
		transform_.rotate.y = MyMath::Leap(nowTurnY_, turnTable[static_cast<uint32_t>(lrDirection_)], t);
	}

	if (map_->GetMapChipTypeByIndex(xIndex - 1, yIndex) == MapChipType::kBlock) {
		if ((map_->GetMapChipPositionByIndex(xIndex - 1, yIndex) - transform_.translate).Length() <= kWith * 0.5f + kBlockWidth * 0.5f) {
			transform_.translate.x -= velocity_.x * engineCore_->GetDeltaTime();
		}
	}

	if (map_->GetMapChipTypeByIndex(xIndex + 1, yIndex) == MapChipType::kBlock) {
		if ((map_->GetMapChipPositionByIndex(xIndex + 1, yIndex) - transform_.translate).Length() <= kWith * 0.5f + kBlockWidth * 0.5f) {
			transform_.translate.x -= velocity_.x * engineCore_->GetDeltaTime();
		}
	}

	if (map_->GetMapChipTypeByIndex(xIndex, yIndex - 1) == MapChipType::kBlock) {
		if ((map_->GetMapChipPositionByIndex(xIndex, yIndex - 1) - transform_.translate).Length() <= kWith * 0.5f + kBlockWidth * 0.5f) {
			transform_.translate.y -= velocity_.y * engineCore_->GetDeltaTime();
			acceleration_.y = 0.0f;
			velocity_.y = 0.0f;
		}
	}
	
	ImGui::Text("center %f", (map_->GetMapChipPositionByIndex(xIndex, yIndex) - transform_.translate).Length());
	ImGui::Text("right %f", (map_->GetMapChipPositionByIndex(xIndex + 1, yIndex) - transform_.translate).Length());
	ImGui::Text("left %f", (map_->GetMapChipPositionByIndex(xIndex - 1, yIndex) - transform_.translate).Length());

	model_.transform_ = transform_;
	model_.Update();
}

void Player::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}

	ImGui::Begin("player");
	ImGui::DragFloat3("transition", &transform_.translate.x);
	ImGui::DragFloat3("velocity", &velocity_.x);
	ImGui::DragFloat3("acceleration", &acceleration_.x);
	uint32_t xIndex = static_cast<uint32_t>(transform_.translate.x / kBlockWidth);
	uint32_t yIndex = kNumBlockVertical - static_cast<uint32_t>(transform_.translate.y / kBlockHeight) - 1;
	ImGui::Text("xIndex:%d yIndex:%d",xIndex, yIndex);
	ImGui::Text("%f %f", map_->GetMapChipPositionByIndex(xIndex, yIndex).x, map_->GetMapChipPositionByIndex(xIndex, yIndex).y);
	ImGui::Text("Deltatime:%f", engineCore_->GetDeltaTime());
	ImGui::End();

	model_.Draw(camera);
}

bool Player::GetIsActive() {
	return isActive_;
}

void Player::SetIsActive(bool isActive) {
	isActive_ = isActive;
}

void Player::SetMap(MapChip* map) {
	map_ = map;
}
