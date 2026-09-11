project "GraphicFramework"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
            "DirectXTex",
            "GraphicEngine",
        }

        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/",
            "%{wks.location}/engine/core/",
        }

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/",
        }


-- Premake 5.0.0-beta2ではusage/usesが利用できないため、
-- GraphicFrameworkの利用に必要なリンク設定をこの機能側へ集約する。
function QFE_UseGraphicFramework()
        links {
                "GraphicFramework",
                "GraphicEngine",
                "EngineCore",
                "DirectXTex",
        }
end
