#pragma once
#include "SceneObject.h"
#include <stack>

#ifdef _DEBUG
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG

#include "Colliders/CollisionManager.h"
#include "Object/Asset/AssetManager.h"
#include "SceneGameObjectGenerator.h"

class EngineCore;

// TODO: シーンでルアスクリプトを作らない
// TODO: シーンマネージャーはシーンの切り替えとシーンの保存・読み込みのみを行う

class SceneManager {
public:
	SceneManager();

public:
	void CreateScene(EngineCore* engineCore, const std::string& sceneName);

public:
	void InitializeScene();
	void UpdateScene();
	void DrawScene();

public:
#ifdef _DEBUG
	void DrawImGui();
	void DrawGizmo(const ImGuizmo::OPERATION& op, const ImGuizmo::MODE& mode, const ImVec2& imageScreenPos, const ImVec2& imageSize);
	void RequestUndo() { requestUndo_ = true; }
	void RequestRedo() { requestRedo_ = true; }
	void PickObjectFromScreen(float relX, float relY);
#endif // _DEBUG

public:
	void SaveScenesToJson(const std::string& filepath);
	void LoadScenesFromJson(const std::string& filepath);
	void LoadScenesLua(const std::string& filename);
	void SwapScene();

public:
	std::string GetCurrentSceneName() const;
	void SetIsRunningScript(bool isRunning) { isRunningScript_ = isRunning; }

private:
#ifdef _DEBUG
	void Undo();
	void Redo();
	void PushUndo();
	bool requestUndo_;
	bool requestRedo_;
	std::stack<nlohmann::json> undoStack_;
	std::stack<nlohmann::json> redoStack_;
#endif // _DEBUG
	nlohmann::json currentSceneData_;

	bool isRequestSwapScene_; // シーンの切り替え要求フラグ
	std::string loadedScenePath_;
	std::unique_ptr<SceneObject> loadedScene_;
	std::unique_ptr<SceneObject> currentScene_;
	std::vector<std::string> sceneFilepath_;
	std::string selectedSceneFilepath_;
	int sceneSelectionIndex_; // 現在選択されているシーンのインデックス
	EngineCore* engineCore_;
	std::string sceneDataDirectorypath_;

private:
//#ifdef _DEBUG
	int scriptSelectionIndex_;
	std::vector<std::string> scriptFilepath_;
	std::string scriptInputFilepath_;
	std::string scriptDirectoryPath_;

	bool isRunningScript_;
	bool startToRunScript_;

	int modelSelectionIndex_;
	std::vector<std::string> modelFilepaths_;
	std::string inputFilepath_;
	std::string ModelDirectoryPath_;

	
	int billboardSelectionIndex_;
	std::vector<std::string> billboardFilepath_;
	std::string billboardInputFilepath_;
	std::string imageDirectoryPath_;

	int spriteSelectionIndex_;
	std::vector<std::string> spriteFilepath_;
	std::string spriteInputFilepath_;
//#endif // _DEBUG

	SceneGameObjectGenerator sceneGameObjectGenerator_;
	CollisionManager collisionManager_;
	AssetManager assetManager_;
	std::string assetFileName_;
};