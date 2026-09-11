project "GameEditor" -- GameEditor
        kind "WindowedApp"
        language "C++"
        debugdir "%{wks.location}"
        files {
            "./**.h",
            "./**.cpp"
        }

        links{
            "EngineCore",
            "GuiFramework",
            "WindowFramework",
            "EngineAssetsFactory",
            "SceneFramework",
            "ApplicationFramework",
            "PhysicsFramework",
            "Camera",
            "ImGui",
            "Components",
            "PhysicsEngine",
            "InputFramework",
        }

        -- グラフィック関連の内部ライブラリはGraphicFramework側で管理する。
        QFE_UseGraphicFramework()

        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/",
            "%{wks.location}/editor/",
            "%{wks.location}/engine/core/",
        }

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/",
            "%{wks.location}/externals/assimp/",
            "%{wks.location}/externals/assimp/include/",
            "%{wks.location}/externals/imgui/"
        }
