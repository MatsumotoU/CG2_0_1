#include "EngineCore.h"

EngineCore::EngineCore() {
	engineStartTime = std::chrono::steady_clock::now();
#ifdef _DEBUG
	DebugLog("[[[EngineStarted]]]");
	systemTimer_.Init();

	initializeRunningTime_ = 0.0f;
	updateRunningTime_ = 0.0f;
	drawRunningTime_ = 0.0f;
#endif // _DEBUG

	lunchConfigFileName_ = "EngineLunchConfig";
	lunchConfig_ = SJN::LoadJsonData(lunchConfigFileName_);
	lunchConfig_.emplace("FpsDB", false);
	lunchConfig_.emplace("PerformanceDB", false);
	lunchConfig_.emplace("AudioSourceDB", false);
	lunchConfig_.emplace("AudioDataDB", false);
	lunchConfig_.emplace("MemoryDB", false);

	lunchConfig_.emplace("SystemSoundVolume",0.3f);

	isDrawFpsDebugWindow_ = lunchConfig_.at("FpsDB").get<bool>();
	isDrawPerformanceDebugWindow_ = lunchConfig_.at("PerformanceDB").get<bool>();
	isDrawAudioSourceDebugWindow_ = lunchConfig_.at("AudioSourceDB").get<bool>();
	isDrawAudioDataDebugWindow_ = lunchConfig_.at("AudioDataDB").get<bool>();
	isDrawMemoryDebugWindow_ = lunchConfig_.at("MemoryDB").get<bool>();

	systemSpundVolume_ = lunchConfig_.at("SystemSoundVolume").get<float>();

	isDrawLunchConfigWindow_ = false;

	loopStopper_.Initialize();
}

EngineCore::~EngineCore() {
	// 機能の終了処理
	textureManager_.Finalize();
	imGuiManager_.EndImGui();
	dxCommon_.ReleaseDirectXObject();
	CloseWindow(winApp_.GetHWND());
}

void EngineCore::Initialize(LPCWSTR windowName, HINSTANCE hInstance, LPSTR lpCmdLine, MSG* msg) {
#ifdef _DEBUG
	systemTimer_.StartTimer();
	DebugLog("Starting EngineInitialize");
#endif // _DEBUG

	lpCmdLine;
	// windowsアプリ管理クラス初期化
	winApp_.CreateGameWindow(windowName);
	msg_ = msg;
	winApp_.SetMSG(msg);
	hInstance;
	// DirectXの初期化
#ifdef _DEBUG
	dxCommon_.SetCommandLine(&lpCmdLine);
#endif // _DEBUG
	// DirectX初期化
	dxCommon_.Initialize(&winApp_);
	// 各ディスクリプタヒープの作成
	rtvDescriptorHeap_.Initialize(dxCommon_.GetDevice(), 3, false);
	srvDescriptorHeap_.Initialize(dxCommon_.GetDevice(), 128, true);
	// DirectXの画面リソースの初期化とRTV登録
	dxCommon_.InitializeScreenResources(rtvDescriptorHeap_.GetRtvDescriptorHeap());

	// Dxc初期化
	shaderCompiler_.InitializeDXC();

	// imGuiManager初期化
	imGuiManager_.Initialize(&winApp_,&dxCommon_,srvDescriptorHeap_.GetSrvDescriptorHeap());
	// テクスチャマネージャの初期化
	textureManager_.Initialize(&dxCommon_, &srvDescriptorHeap_);
	graphicsCommon_.Initialize(this);
	// fps監視機能初期化
	fpsCounter_.Initialize();
	fpsCounter_.Update();

	// グラフ描画機能の初期化
	graphRenderer_.Initialize(this);

	// 音声機能の初期化
	audioManager_.Initialize();
	audioResourceManager_.Initialize(this);
	audioSourceBinder_.Initialize(this);
	audioPlayer_.Initialize(this);
	chiptune_.Initialize(this);

	// 入力機能の初期化
	inputManager_.Initialize(&winApp_, hInstance);

	// オフスクリーンのスプライト生成
	offscreen_.Initialize(this,static_cast<float>(winApp_.kWindowWidth), static_cast<float>(winApp_.kWindowHeight));
	offscreen_.material_.materialData_->enableLighting = false;

	camera_.Initialize(&winApp_);

#ifdef _DEBUG
	systemTimer_.StopTimer();
	initializeRunningTime_ = systemTimer_.GetElapsedTime();
	DebugLog(std::format("Complete InitializeEngine: {}s", initializeRunningTime_));
	
#endif // _DEBUG

	loopStopper_.Initialize();
}

void EngineCore::Update() {
#ifdef _DEBUG
	systemTimer_.StartTimer();
	DebugLog("==========Starting Update==========");
#endif // _DEBUG

	// エンジンの更新処理
	imGuiManager_.BeginFrame();
	fpsCounter_.Update();
	inputManager_.Update();
	xController_.Update();

#ifdef _DEBUG
	systemTimer_.StopTimer();
	updateRunningTime_ = systemTimer_.GetElapsedTime();
	DebugLog(std::format("Update time : {}s", updateRunningTime_));
	DebugLog("-------------End Update-------------");
#endif // _DEBUG

	loopStopper_.Update();
}

void EngineCore::PreDraw() {
#ifdef _DEBUG
	systemTimer_.StartTimer();
	DebugLog("-------------Starting Draw-------------");
#endif // _DEBUG

	// オフスクリーンのバリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = dxCommon_.GetOffscreenResource();
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// バリア張る
	dxCommon_.GetCommandList()->ResourceBarrier(1, &barrier);

	dxCommon_.PreDraw();
	
	textureManager_.PreDraw();

	// 深度の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = graphicsCommon_.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetOffscreenRtvHandles(), false, &dsvHandl);
	//dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	graphRenderer_.PreDraw();
}

void EngineCore::PostDraw() {

#ifdef _DEBUG
	DrawEngineMenu();
#endif // _DEBUG

	graphRenderer_.PostDraw();

	// オフスクリーンのバリア
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはトランジション
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにする
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを貼る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = dxCommon_.GetOffscreenResource();
	// 遷移前（現在）のリソースステート
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	// バリア張る
	dxCommon_.GetCommandList()->ResourceBarrier(1, &barrier);

	// 深度の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = graphicsCommon_.GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetOffscreenRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->OMSetRenderTargets(1, dxCommon_.GetRtvHandles(), false, &dsvHandl);
	dxCommon_.GetCommandList()->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	offscreen_.DrawSprite(offscreenTransform_,uvTransform_,textureManager_.GetOffscreenSrvHandleGPU(),&camera_);

	textureManager_.PostDraw();
	imGuiManager_.EndFrame();
	dxCommon_.PostDraw();
	textureManager_.ReleaseIntermediateResources();

#ifdef _DEBUG
	systemTimer_.StopTimer();
	drawRunningTime_ = systemTimer_.GetElapsedTime();
	DebugLog(std::format("Draw time : {}s", drawRunningTime_));
	DebugLog("============End Draw============");
#endif // _DEBUG
}

uint32_t EngineCore::LoadSoundData(const std::string& filePath, const std::string& fileName) {
	audioResourceManager_.LoadAudioResource(filePath, fileName);
	return audioResourceManager_.GetAudioDataHandle(fileName);
}

WinApp* EngineCore::GetWinApp() {
	return &winApp_;
}

DirectXCommon* EngineCore::GetDirectXCommon() {
	return &dxCommon_;
}

ImGuiManager* EngineCore::GetImGuiManager() {
	return &imGuiManager_;
}

TextureManager* EngineCore::GetTextureManager() {
	return &textureManager_;
}

GraphicsCommon* EngineCore::GetGraphicsCommon() {
	return &graphicsCommon_;
}

XAudioCore* EngineCore::GetAudioManager() {
	return &audioManager_;
}

Audio3D* EngineCore::GetAudio3D() {
	return &audio3D_;
}

DirectInputManager* EngineCore::GetInputManager() {
	return &inputManager_;
}

Sprite* EngineCore::GetOffscreen() {
	return &offscreen_;
}

FramePerSecond* EngineCore::GetFpsCounter() {
	return &fpsCounter_;
}

RtvDescriptorHeap* EngineCore::GetRtvDescriptorHeap() {
	return &rtvDescriptorHeap_;
}

SrvDescriptorHeap* EngineCore::GetSrvDescriptorHeap() {
	return &srvDescriptorHeap_;
}

GraphRenderer* EngineCore::GetGraphRenderer() {
	return &graphRenderer_;
}

XInputController* EngineCore::GetXInputController() {
	return &xController_;
}

AudioResourceManager* EngineCore::GetAudioResourceManager() {
	return &audioResourceManager_;
}

AudioSourceBinder* EngineCore::GetAudioSourceBinder() {
	return &audioSourceBinder_;
}

AudioPlayer* EngineCore::GetAudioPlayer() {
	return &audioPlayer_;
}

Chiptune* EngineCore::GetChiptune() {
	return &chiptune_;
}

ShaderCompiler* EngineCore::GetShaderCompiler() {
	return &shaderCompiler_;
}

LoopStoper* EngineCore::GetLoopStopper() {
	return &loopStopper_;
}

float EngineCore::GetDeltaTime() {
	// FPSが0でない場合はFPSを基にデルタタイムを返す
	if (fpsCounter_.GetFps() != 0.0f) {

		// FPSが変動している場合は60FPS想定でデルタタイムを返す
		if (fabsf(fpsCounter_.GetAverageFps() - fpsCounter_.GetFps()) >= 5.0f) {
			return 1.0f / 60.0f;
		}
		// FPSが安定している場合はそのFPSを基にデルタタイムを返す
		return 1.0f / fpsCounter_.GetFps();
	}
	return 0.0f;
}

#ifdef _DEBUG
void EngineCore::DrawEngineMenu() {
	// エンジンのウィンドウと差別化用のスタイル変更
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
	if (ImGui::BeginMainMenuBar()) {
		// 編集機能
		if (ImGui::BeginMenu("Edit")) {
			// 起動構成
			if (ImGui::Button("Lunch Config")) {
				chiptune_.PlayMainSquareWave(GermanNote::A, 4, 0.1f, systemSpundVolume_);
				isDrawLunchConfigWindow_ = true;
			}
			ImGui::EndMenu();
		}

		// エンジンの詳細情報
		if (ImGui::BeginMenu("View")) {
			// 一般
			if (ImGui::TreeNode("General")) {
				ImGui::Checkbox("FPS", &isDrawFpsDebugWindow_);
				ImGui::Checkbox("PERFOFMANCE", &isDrawPerformanceDebugWindow_);
				ImGui::Checkbox("MEMORY", &isDrawMemoryDebugWindow_);
				ImGui::TreePop();
			}
			// 音声
			if (ImGui::TreeNode("Audio")) {
				ImGui::Checkbox("AUDIODATA", &isDrawAudioDataDebugWindow_);
				ImGui::Checkbox("AUDIOSOURCE", &isDrawAudioSourceDebugWindow_);
				ImGui::TreePop();
			}
			ImGui::EndMenu();
		}

		ImGui::Spacing();
		// 一時停止
		if (ImGui::Button("Stop")) {
			if (loopStopper_.GetIsStopping()) {
				//chiptune_.PlayMainSquareWave(GermanNote::A, 1, 0.1f, systemSpundVolume_);
			} else {
				//chiptune_.PlayMainSquareWave(GermanNote::A, 4, 0.1f, systemSpundVolume_);
				loopStopper_.SetIsStopping(true);
			}
		}
		// 再生
		if (ImGui::Button("Start")) {
			if (!loopStopper_.GetIsStopping()) {
				//chiptune_.PlayMainSquareWave(GermanNote::A, 1, 0.1f, systemSpundVolume_);
			} else {
				//chiptune_.PlayMainSquareWave(GermanNote::A, 4, 0.1f, systemSpundVolume_);
				loopStopper_.SetIsStopping(false);
			}
		}
		if (loopStopper_.GetIsStopping()) {
			ImGui::Text("!!!Update Stopping Now!!!");
		}

		ImGui::EndMainMenuBar();
	}

	// 場所が分かるようにアウトラインを出す
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

	//* Editタブ *//
	if (isDrawLunchConfigWindow_) {
		ImGui::Begin("LunchConfig", &isDrawLunchConfigWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		if (ImGui::Button("Save")) {
			chiptune_.PlayMainSquareWave(GermanNote::A, 4, 0.1f, systemSpundVolume_);
			SaveLunchConfig();
		}

		// 起動時に実行されるもの
		if (ImGui::TreeNode("StratSetup")) {
			// 起動時に表示しておくデバッグウィンドウ
			if (ImGui::TreeNode("Window")) {

				// 一般
				if (ImGui::TreeNode("General")) {
					if (ImGui::Button("FPS")) {
						lunchConfig_.at("FpsDB") = !lunchConfig_.at("FpsDB").get<bool>();
					}
					ImGui::SameLine();
					ImGui::Text("%s", lunchConfig_.at("FpsDB").get<bool>() ? "True" : "False");

					if (ImGui::Button("PERFOFMANCE")) {
						lunchConfig_.at("PerformanceDB") = !lunchConfig_.at("PerformanceDB").get<bool>();
					}
					ImGui::SameLine();
					ImGui::Text("%s", lunchConfig_.at("PerformanceDB").get<bool>() ? "True" : "False");

					if (ImGui::Button("MEMORY")) {
						lunchConfig_.at("MemoryDB") = !lunchConfig_.at("MemoryDB").get<bool>();
					}
					ImGui::SameLine();
					ImGui::Text("%s", lunchConfig_.at("MemoryDB").get<bool>() ? "True" : "False");
					ImGui::TreePop();
				}
				// 音声
				if (ImGui::TreeNode("Audio")) {
					if (ImGui::Button("AUDIODATA")) {
						lunchConfig_.at("AudioDataDB") = !lunchConfig_.at("AudioDataDB").get<bool>();
					}
					ImGui::SameLine();
					ImGui::Text("%s", lunchConfig_.at("AudioDataDB").get<bool>() ? "True" : "False");

					if (ImGui::Button("AUDIOSOURCE")) {
						lunchConfig_.at("AudioSourceDB") = !lunchConfig_.at("AudioSourceDB").get<bool>();
					}
					ImGui::SameLine();
					ImGui::Text("%s", lunchConfig_.at("AudioSourceDB").get<bool>() ? "True" : "False");
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}

	//* Viewタブ *//
	// Fpsデバッグ情報
	if (isDrawFpsDebugWindow_) {
		ImGui::Begin("FPS",&isDrawFpsDebugWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("AverageFps = %f", fpsCounter_.GetAverageFps());
		ImGui::Text("nowFps = %f", fpsCounter_.GetFps());
		ImGui::PlotLines(
			"Fps",
			fpsCounter_.GetFpsSample(),
			static_cast<int>(fpsCounter_.GetFpsSamplerNum()),
			0, NULL,
			0, 128, ImVec2(0, 128.0f));
		ImGui::End();
	}

	// 負荷監視
	if (isDrawPerformanceDebugWindow_) {
		ImGui::Begin("PERFOFMANCE", &isDrawPerformanceDebugWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("InitializeRunningTime = %fs", initializeRunningTime_);
		ImGui::Text("UpdateRunningTime = %fs", updateRunningTime_);
		ImGui::Text("DrawRunningTime = %fs", drawRunningTime_);
		ImGui::Text("Deltatime = %fs", GetDeltaTime());
		ImGui::Separator();

		float total = updateRunningTime_ + drawRunningTime_;
		float updateRate = (total > 0.0f) ? (updateRunningTime_ / total) : 0.0f;
		float drawRate = (total > 0.0f) ? (drawRunningTime_ / total) : 0.0f;

		// 比率をプログレスバーで可視化
		ImGui::Text("Process Rate");
		ImGui::ProgressBar(updateRate, ImVec2(100, 0), "Update");
		ImGui::SameLine();
		ImGui::Text("%.1f%%", updateRate * 100.0f);
		ImGui::ProgressBar(drawRate, ImVec2(100, 0), "Draw");
		ImGui::SameLine();
		ImGui::Text("%.1f%%", drawRate * 100.0f);
		ImGui::End();
	}

	if (isDrawMemoryDebugWindow_) {
		ImGui::Begin("MEMORY", &isDrawMemoryDebugWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		memWatcher_.DrawImGui();
		ImGui::End();
	}

	// オーディオソース監視
	if (isDrawAudioSourceDebugWindow_) {
		ImGui::Begin("AUDIOSOURCE", &isDrawAudioSourceDebugWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Loaded AudioSources");
		ImGui::Separator();
		audioSourceBinder_.DrawImGui();
		ImGui::End();
	}
	// サウンドデータ監視
	if (isDrawAudioDataDebugWindow_) {
		ImGui::Begin("AUDIODATA", &isDrawAudioDataDebugWindow_, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		audioResourceManager_.DrawImGui();
		ImGui::End();
	}

	// エンジンのウィンドウと差別化用のスタイル変更
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void EngineCore::SaveLunchConfig() {
	SJN::SaveJsonData(lunchConfigFileName_,lunchConfig_);
}
#endif // _DEBUG