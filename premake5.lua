dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

dependencies.load()

workspace "w3x"
	startproject "w3x"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

	configurations {
		"Debug",
		"Release",
	}

	architecture "x64"
	platforms "x64"

	buildoptions "/std:c++20"
	systemversion "latest"
	symbols "On"
	staticruntime "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"
	
	flags {
		"NoIncrementalLink",
		"NoMinimalRebuild",
		"MultiProcessorCompile",
		"No64BitChecks"
	}

	filter "platforms:x64"
		defines {
			"_WINDOWS",
			"WIN32",
		}

	filter "configurations:Release"
		optimize "Full"
		buildoptions "/Os"

		defines {
			"NDEBUG",
		}

		flags {
			"FatalCompileWarnings",
		}

	filter "configurations:Debug"
		optimize "Debug"

		defines {
			"DEBUG",
			"_DEBUG",
		}

	filter {}

	project "w3x"
		kind "WindowedApp"
		language "C++"

		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"

		files {
			"./src/**.rc",
			"./src/**.hpp",
			"./src/**.cpp",
			"./src/resources/**.*"
		}

		includedirs {
			"./src",
			"%{prj.location}/src",
		}

		resincludedirs {
			"$(ProjectDir)src"
		}

		if _OPTIONS["copy-to"] then
			postbuildcommands {
				"copy /y \"$(TargetDir)*.exe\" \"" .. _OPTIONS["copy-to"] .. "\""
			}
		end

		dependencies.imports()

	group "Dependencies"
		dependencies.projects()
