Librw = os.getenv("LIBRW")

workspace "librwgta"
	location "build"
	language "C++"

	configurations { "Release", "Debug" }
	filter { "system:windows" }
		configurations { "ReleaseStatic" }
		platforms { "win-x86-null", "win-x86-gl3", "win-x86-d3d9",
			"win-amd64-null", "win-amd64-gl3", "win-amd64-d3d9" }
	filter { "system:linux" }
		platforms { "linux-x86-null", "linux-x86-gl3",
			"linux-amd64-null", "linux-amd64-gl3" }
	-- TODO: ps2
	filter {}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
	filter "configurations:Release*"
		defines { "NDEBUG" }
		optimize "On"
	filter "configurations:ReleaseStatic"
		flags { "StaticRuntime" }

	filter { "platforms:*null" }
		defines { "RW_NULL" }
	filter { "platforms:*gl3" }
		defines { "RW_GL3" }
	filter { "platforms:*d3d9" }
		defines { "RW_D3D9" }
	filter { "platforms:*ps2" }
		defines { "RW_PS2" }

	filter { "platforms:*amd64*" }
		architecture "x86_64"
	filter { "platforms:*x86*" }
		architecture "x86"

	filter { "platforms:win*" }
		system "windows"
	filter { "platforms:linux*" }
		system "linux"

	filter "action:vs*"
		buildoptions { "/wd4996" }
		
	filter {}

	Libdir = "lib/%{cfg.platform}/%{cfg.buildcfg}"
	Bindir = "bin/%{cfg.platform}/%{cfg.buildcfg}"

	libdirs { path.join(Librw, Libdir) }
	includedirs { Librw }
	includedirs { "src" }

project "librwgta"
	kind "StaticLib"
	targetname "rwgta"
	targetdir (Libdir)
	files { "src/*.*" }

function tool(dir)
	targetdir (Bindir)
	files { path.join("tools", dir, "*") }
	libdirs { Libdir }
	links { "rw", "librwgta" }
	findlibs()
end

function findlibs()
	filter { "platforms:*gl3" }
		links { "GL", "GLEW", "glfw" }
	filter { "platforms:*d3d9" }
		links { "d3d9" }
	filter {}
end

project "convdff"
	tool("convdff")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

project "convtxd"
	tool("convtxd")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

project "lcsconv"
	tool("storiesconv")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }
	defines { "LCS" }

project "vcsconv"
	tool("storiesconv")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }
	defines { "VCS" }

project "coltest"
	tool("coltest")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

project "d3d9test"
	tool("d3d9test")
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	removeplatforms { "*gl3", "*null" }
	links { "winmm" }

project "IIItest"
	tool("IIItest")
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	removeplatforms { "*null" }
	files { path.join("tools/IIItest/src/*") }

