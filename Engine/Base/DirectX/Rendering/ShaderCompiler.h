#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <map>

#include "ShaderReflection.h"
#include "Base/MyString.h"

class ShaderCompiler final {
public:
	ShaderCompiler();
	~ShaderCompiler();

public:
	/// <summary>
	/// DXCを初期化します
	/// </summary>
	void InitializeDXC();

	/// <summary>
	/// シェーダーをコンパイルする
	/// </summary>
	/// <param name="filePath">コンパイル対象のhlslファイル名</param>
	/// <param name="profile">コンパイラに使用するプロファイル</param>
	/// <param name="dxUtils"></param>
	/// <param name="dxcCompiler"></param>
	/// <param name="includeHandler"></param>
	/// <returns></returns>
	IDxcBlob* CompileShader(const std::wstring& filePath,const wchar_t* profile);

	/// <summary>
	/// シェーダーバイトコードからリフレクション情報を取得する
	/// </summary>
	/// <param name="shaderBlob">コンパイル済みシェーダー(IDxcBlob)</param>
	/// <param name="reflection">リフレクション情報格納先</param>
	/// <returns>成功した場合true</returns>
	bool ReflectShader(IDxcBlob* shaderBlob, ShaderReflection& reflection);

private:
	std::map<std::wstring, IDxcBlob*> iDxcBlobMap_;
	IDxcUtils* dxcUtils_;
	IDxcCompiler3* dxcCompiler_;
	IDxcIncludeHandler* includeHandler_;
};