// Debug
#ifdef _DEBUG
#include "Engine/Base/Windows/WinAppDebugCore.h"
#include "Engine/Base/DirectX/DirectX12DebugCore.h"
#endif // _DEBUG

// 現状ないと動かないコアたち
#include "Engine/Base/EngineCore.h"

// windowsアプリでのエントリーポイント(main関数) 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int) {
	// * ゲーム以前の設定 * //
#ifdef _DEBUG
	WinAppDebugCore winAppDbgCore(lpCmdLine);
	DirectX12DebugCore directXDbgCore;
#endif // _DEBUG

	// エンジンの初期化
	MSG msg{};
	EngineCore engineCore;
	engineCore.Initialize(L"LE2A_14_マツモト_ユウタ_AL3", hInstance, lpCmdLine,&msg);

	// ウィンドウのXボタンが押されるまでループ
	while (engineCore.GetWinApp()->GetIsWindowQuit()) {
		// windowにメッセージが基底たら最優先で処理される
		if (engineCore.GetWinApp()->GetCanLoop()) {
			// === Update ===
			engineCore.Update();

			// === Draw ===
			engineCore.PreDraw();

			// === EndDraw ===
			engineCore.PostDraw();
		}
	}
	return 0;
}