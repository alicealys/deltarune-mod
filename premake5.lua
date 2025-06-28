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

workspace "deltarune-mod"
	location "./build"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	targetname "%{prj.name}"

	configurations { "Debug", "Release", }

	language "C++"
	cppdialect "C++20"

	architecture "x64"
	platforms "x64"

	systemversion "latest"
	symbols "On"
	staticruntime "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"

	flags
	{
		"NoIncrementalLink",
		"MultiProcessorCompile",
	}
	
	filter "configurations:Release"
		optimize "Full"
		defines { "NDEBUG" }
	filter {}

	filter "configurations:Debug"
		optimize "Debug"
		defines { "DEBUG", "_DEBUG" }
	filter {}

	startproject "deltarune-mod"
		project "deltarune-mod"
			kind "SharedLib"
			language "C++"

			targetname "dinput8"

			files 
			{
				"./src/**.h",
				"./src/**.hpp",
				"./src/**.cpp",
				"./src/**.rc",
			}

			includedirs 
			{
				"%{prj.location}/src",
				"./src"
			}

			resincludedirs 
			{
				"$(ProjectDir)src"
			}
		
			pchheader "stdinc.hpp"
			pchsource "src/stdinc.cpp"

			dependencies.imports()

			if _OPTIONS["copy-to"] then
				postbuildcommands {"copy /y \"$(TargetPath)\" \"" .. _OPTIONS["copy-to"] .. "\""}
			end

		group "Dependencies"
			dependencies.projects()