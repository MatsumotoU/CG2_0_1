#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <source_location>
#include <vector>

class MyDebugLog final {
public:
	// シングルトン
	static MyDebugLog* GetInstatnce();

public:
	void Initialize();
	void Finalize();
	void Log(const std::string& message, const std::source_location& location = std::source_location::current());

public:
	std::vector<std::string>* GetLog();

private:
	std::ofstream logStream_;
	std::string logFilePath_;
	std::vector<std::string> log_;

private: // シングルトン用
	MyDebugLog() = default;
	~MyDebugLog() = default;
	MyDebugLog(const MyDebugLog&) = delete;
	MyDebugLog& operator=(const MyDebugLog&) = delete;
};

/// <summary>
/// このヘッダーを読み込んでいれば使えるやつ
/// </summary>
/// <param name="message"></param>
void DebugLog(const std::string& message, const std::source_location& location = std::source_location::current());