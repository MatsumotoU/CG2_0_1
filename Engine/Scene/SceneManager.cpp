#include "SceneManager.h"
#include "Base/EngineCore.h"
#ifdef _DEBUG
#include "Base/DirectX/ImGuiManager.h"
#include "Utility/FileLoader.h"
#include "Base/MyDebugLog.h"
#endif // _DEBUG

static char saveFileName[256] = "";
static int selectedIndex = -1;

SceneManager::SceneManager() {
	isRequestSwapScene_ = false;
}

void SceneManager::CreateScene(EngineCore* engineCore, const std::string& sceneName) {
	engineCore_ = engineCore;
	currentScene_ = std::make_unique<SceneObject>(engineCore, sceneName);
	sceneDataDirectorypath_ = "Resources/Scenes";
	sceneFilepath_.clear();
	sceneSelectionIndex_ = 0;
	sceneFilepath_ = FileLoader::GetFilesWithExtension(sceneDataDirectorypath_, ".json");
	if (sceneFilepath_.size() > 0) {
		selectedSceneFilepath_ = sceneFilepath_[0]; // 最初のシーンを選択
	} 
#ifdef _DEBUG
	modelSelectionIndex_ = 0;
	inputFilepath_; // デフォルトのモデルデータパス
	ModelDirectoryPath_ = "Resources"; // デフォルトのモデルデータパス

	// モデルファイルのパスを取得
	modelFilepaths_ = FileLoader::GetFilesWithExtension(ModelDirectoryPath_, ".obj");
	if (modelFilepaths_.size() > 0) {
		inputFilepath_ = modelFilepaths_[0]; // 最初のモデルを選択
	}

	billboardSelectionIndex_ = 0;
	
	billboardFilepath_ = FileLoader::GetFilesWithExtension(ModelDirectoryPath_, ".png");
	if (billboardFilepath_.size() > 0) {
		billboardInputFilepath_ = billboardFilepath_[0]; // 最初のビルボードを選択
	}
#endif
}

void SceneManager::InitializeScene() {
	if (currentScene_) {
		currentScene_->Initialize();
	}
}

void SceneManager::UpdateScene() {
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::DrawScene() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::SaveScenesToJson(const std::string& filepath) {
#ifdef _DEBUG
	DebugLog("===Start save scenes!===");
#endif // _DEBUG

	nlohmann::json root;
	if (currentScene_) {
		root["scenes"].push_back(currentScene_->Serialize());
	}

	std::ofstream ofs(filepath);
	if (!ofs) {
		// ディレクトリがなければ作成
		std::filesystem::path path(filepath);
		std::filesystem::create_directories(path.parent_path());
		// 再度ファイルを開く
		ofs.open(filepath);
	}
	assert(ofs);
	ofs << root.dump(4); // インデント付きで出力
	ofs.close();

#ifdef _DEBUG
	DebugLog("===End save scenes===");
#endif // _DEBUG
}

void SceneManager::LoadScenesFromJson(const std::string& filepath) {
	engineCore_->GetDirectXCommon()->WaitForGpu();

	std::ifstream ifs;
	ifs.open(filepath);
	assert(ifs);

	nlohmann::json root;
	ifs >> root;
	if (root.contains("scenes")) {
		// 例: 1つだけロード
		loadedScene_ = SceneObject::Deserialize(root["scenes"][0], engineCore_,ModelDirectoryPath_);
		isRequestSwapScene_ = true;
	}
}

void SceneManager::SwapScene() {
	if (isRequestSwapScene_) {
		isRequestSwapScene_ = false;
		currentScene_ = std::move(loadedScene_);
		currentScene_->Initialize();
	}
}

std::string SceneManager::GetCurrentSceneName() const {
	return currentScene_ ? currentScene_->GetSceneName() : "UnnamedScene";
}

#ifdef _DEBUG
void SceneManager::DrawImGui() {
	if (ImGui::BeginTabBar("SceneTab")) {
		// シーンの保存と読み込みタブ
		if (ImGui::BeginTabItem("Scenes")) {
			if (ImGui::Button("SaveScene")) {
				// 入力が空の場合はデフォルト名を使う
				std::string fileName = strlen(saveFileName) > 0 ? saveFileName : GetCurrentSceneName();
				std::string filepath = sceneDataDirectorypath_ + "/" + fileName + ".json";
				if (currentScene_) {
					currentScene_->SetSceneName(fileName);
				}
				SaveScenesToJson(filepath);
			}
			ImGui::SameLine();
			ImGui::InputText("Save As", saveFileName, IM_ARRAYSIZE(saveFileName));

			if (isRequestSwapScene_) {
				ImGui::Text("Scene has been changed. Please reload.");
			} else {
				if (ImGui::Button("LoadScene")) {
					std::string filepath = sceneDataDirectorypath_ + "/" + selectedSceneFilepath_;
					LoadScenesFromJson(filepath);
				}
			}
			ImGui::SameLine();
			// ImGui::Combo用にconst char*配列を作成
			std::vector<const char*> items;
			for (const auto& f : sceneFilepath_) {
				items.push_back(f.c_str());
			}
			if (ImGui::Combo("Model", &sceneSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
				// 選択が変わったときの処理
				selectedSceneFilepath_ = sceneFilepath_[sceneSelectionIndex_];
			}

			if (ImGui::Button("ReloadScene File")) {
				// モデルファイルのパスを再取得
				sceneFilepath_.clear();
				sceneSelectionIndex_ = 0;
				sceneFilepath_ = FileLoader::GetFilesWithExtension(sceneDataDirectorypath_, ".json");
				if (sceneFilepath_.size() > 0) {
					selectedSceneFilepath_ = sceneFilepath_[0]; // 最初のシーンを選択
				}
			}

			ImGui::EndTabItem();
		}

		// シーンオブジェクトのタブ
		if (ImGui::BeginTabItem("SceneObjects")) {
			if (currentScene_) {
				auto& gameObjects = currentScene_->GetGameObjects();
				BaseGameObject* objectToDelete = nullptr;
				int deleteIndex = -1;

				// 縦スクロール可能な領域を作成（幅300, 高さ200など適宜調整）
				ImGui::Text("Game Objects in Scene: %s", currentScene_->GetSceneName().c_str());
				ImGui::BeginChild("ObjectListChild", ImVec2(300, 200), true);

				for (size_t i = 0; i < gameObjects.size(); ++i) {
					auto& gameObject = gameObjects[i];
					std::string label = "[" + std::to_string(i) + "] " + gameObject->GetName() + "##" + std::to_string(i);
					bool isSelected = (selectedIndex == static_cast<int>(i));
					if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(200, 0))) {
						selectedIndex = static_cast<int>(i);
					}
					ImGui::SameLine();
					std::string delLabel = "x##" + std::to_string(i);
					if (ImGui::Button(delLabel.c_str())) {
						objectToDelete = gameObject.get();
						deleteIndex = static_cast<int>(i);
					}
				}
				ImGui::EndChild();

				// ループ後に削除
				if (objectToDelete) {
					currentScene_->DeleteModel(objectToDelete);
					if (selectedIndex == deleteIndex) {
						selectedIndex = -1;
					} else if (selectedIndex > deleteIndex) {
						selectedIndex--;
					}
				}

				// 詳細ウィンドウ
				if (selectedIndex >= 0 && selectedIndex < static_cast<int>(gameObjects.size())) {
					ImGui::Begin("Object Detail", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
					gameObjects[selectedIndex]->DrawImGui();
					ImGui::End();
				}
			} else {
				ImGui::Text("No scene loaded.");
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SceneEdit")) {
			if (ImGui::Button("Reload File")) {
				// モデルファイルのパスを再取得
				modelFilepaths_.clear(); // 既存のパスをクリア
				modelFilepaths_ = FileLoader::GetFilesWithExtension(ModelDirectoryPath_, ".obj");
				if (modelFilepaths_.size() > 0) {
					inputFilepath_ = modelFilepaths_[0];
					modelSelectionIndex_ = 0; // 最初のモデルを選択
				} else {
					inputFilepath_.clear(); // ファイルがない場合はクリア
				}
			}

			// モデル追加処理
			ImGui::Text("Model Directory: %s", ModelDirectoryPath_.c_str());
			ImGui::Spacing();
			if (modelFilepaths_.size() > 0) {
				if (ImGui::Button("Add##AddModel")) {
					if (currentScene_) {
						currentScene_->AddModel(ModelDirectoryPath_, inputFilepath_);
					}
				}
				ImGui::SameLine();
				// ImGui::Combo用にconst char*配列を作成
				std::vector<const char*> items;
				for (const auto& f : modelFilepaths_) {
					items.push_back(f.c_str()); 
				}
				if (ImGui::Combo("Model", &modelSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
					// 選択が変わったときの処理
					inputFilepath_ = modelFilepaths_[modelSelectionIndex_];
				}
			} else {
				ImGui::Text("No model files found in %s", ModelDirectoryPath_.c_str());
			}

			// ビルボード追加処理
			ImGui::Text("Billboard Directory: Resources");
			ImGui::Spacing();
			if (billboardFilepath_.size() > 0) {
				if (ImGui::Button("Add##AddBillboard")) {
					if (currentScene_) {
						currentScene_->AddBillboard("Resources", billboardInputFilepath_);
					}
				}
				ImGui::SameLine();
				// ImGui::Combo用にconst char*配列を作成
				std::vector<const char*> items;
				for (const auto& f : billboardFilepath_) {
					items.push_back(f.c_str());
				}
				if (ImGui::Combo("Billboard", &billboardSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
					// 選択が変わったときの処理
					billboardInputFilepath_ = billboardFilepath_[billboardSelectionIndex_];
				}
			} else {
				//ImGui::Text("No model files found in %s", .c_str());
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
#endif // _DEBUG