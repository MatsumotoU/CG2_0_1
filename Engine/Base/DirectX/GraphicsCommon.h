#pragma once
#include "PipelineStateObject.h"
#include "DepthStencil.h"

class EngineCore;

class GraphicsCommon final {
public:
	void Initialize(EngineCore* engineCore);
public:
	/// <summary>
	/// 一般的な三角形のPSOを取得します
	/// </summary>
	/// <param name="blendmode">ブレンドモード</param>
	/// <returns></returns>
	PipelineStateObject* GetTrianglePso(BlendMode blendmode) { return &trianglePso_[static_cast<uint32_t>(blendmode)]; }
	/// <summary>
	/// 一般的な三角形のPSOを取得します
	/// </summary>
	/// <param name="blendmode">ブレンドモード</param>
	/// <returns></returns>
	PipelineStateObject* GetLinePso(BlendMode blendmode) { return &linePso_[static_cast<uint32_t>(blendmode)]; }
	/// <summary>
	/// 一般的な三角形のPSOを取得します
	/// </summary>
	/// <param name="blendmode">ブレンドモード</param>
	/// <returns></returns>
	PipelineStateObject* GetPointPso(BlendMode blendmode) { return &pointPso_[static_cast<uint32_t>(blendmode)]; }

	PipelineStateObject* GetPrimitivePso(BlendMode blendmode) { return &primitivePso_[static_cast<uint32_t>(blendmode)]; }

	PipelineStateObject* GetParticlePso(BlendMode blendmode) { return &particlePso_[static_cast<uint32_t>(blendmode)]; }

	PipelineStateObject* GetGrayScalePso() { return &grayScaleTrianglePso_; }

	DepthStencil* GetDepthStencil() { return &depthStencil_; }

private: // メンバ変数
	DepthStencil depthStencil_;

	// ルートパラメータ;
	RootParameter normalGameObjectRootParameter_;
	RootParameter particleRootParameter_;
	RootParameter primitiveRootParameter_;

	// PSO
	PipelineStateObject trianglePso_[kCountOfBlendMode];
	PipelineStateObject linePso_[kCountOfBlendMode];
	PipelineStateObject pointPso_[kCountOfBlendMode];

	PipelineStateObject primitivePso_[kCountOfBlendMode];

	PipelineStateObject particlePso_[kCountOfBlendMode];

	PipelineStateObject grayScaleTrianglePso_;
};