#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include "../Math/VerTexData.h"
#include "../Math/Transform.h"

class DirectXCommon;
class TextureManager;
class ImGuiManager;// 削除予定

class PipelineStateObject;

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

class Camera;

class Sprite {
public:
	Sprite(DirectXCommon* dxCommon, TextureManager* textureManager, ImGuiManager* imguiManager,float width,float hight,PipelineStateObject* pso);
	~Sprite();

public:
	void DrawSprite(const Transform& transform, int32_t textureHandle,Camera* camera);
	void DrawSprite(const Transform& transform, const Transform& uvTransform, int32_t textureHandle, Camera* camera);

public:
	MaterialResource material_;
	DirectionalLightResource directionalLight_;
	WVPResource wvp_;	

private:
	DirectXCommon* dxCommon_;
	TextureManager* textureManager_;
	ImGuiManager* imGuiManager_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	uint32_t* indexData_;

	PipelineStateObject* pso_;
};