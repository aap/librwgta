Librw = os.getenv("LIBRW")
GLEWdir = "C:/Users/aap/src/glew-2.1.0"
GLFW64dir = "C:/Users/aap/src/glfw-3.2.1.bin.WIN64"
Zlibdir = "C:/Users/aap/src/zlib-1.2.11"

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

	filter { "platforms:win*gl3" }
		includedirs { path.join(GLEWdir, "include") }
		includedirs { path.join(GLFW64dir, "include") }

	filter "action:vs*"
		buildoptions { "/wd4996", "/wd4244" }
		
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
	filter { "platforms:linux*gl3" }
		links { "GL", "GLEW", "glfw" }
	filter { "platforms:win*gl3" }
		defines { "GLEW_STATIC" }
	filter { "platforms:win-amd64-gl3" }
		libdirs { path.join(GLEWdir, "lib/Release/x64") }
		libdirs { path.join(GLFW64dir, "lib-vc2015") }
	filter { "platforms:win-x86-gl3" }
		libdirs { path.join(GLEWdir, "lib/Release/Win32") }
	filter { "platforms:win*gl3" }
		links { "glew32s", "glfw3", "opengl32" }
	filter { "platforms:*d3d9" }
		links { "d3d9", "Xinput9_1_0" }
	filter {}
end

function skeleton()
	files { path.join(Librw, "skeleton/*.cpp"), path.join(Librw, "skeleton/*.h") }
	files { path.join(Librw, "skeleton/imgui/*.cpp"), path.join(Librw, "skeleton/imgui/*.h") }
	includedirs { path.join(Librw, "skeleton") }
end

function skeltool(dir)
	targetdir (Bindir)
	files { path.join("tools", dir, "*.cpp"),
	        path.join("tools", dir, "*.h") }
	vpaths {
		{["src"] = { path.join("tools", dir, "*") }},
		{["skeleton"] = { "skeleton/*" }},
	}
	skeleton()
	debugdir ( path.join("tools", dir) )
	includedirs { "." }
	libdirs { Libdir }
	links { "rw", "librwgta" }
	findlibs()
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

project "lcsview"
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	skeltool("storiesview")
	includedirs { "tools/storiesconv" }
	files { "tools/storiesconv/relocchunk.cpp" }
	files { "tools/storiesconv/rsl.cpp" }
	files { "tools/storiesconv/rslconv.cpp" }
	files { "tools/storiesconv/hash.cpp" }
	includedirs { Zlibdir }
	libdirs { Zlibdir }
	links { "zlib" }
	removeplatforms { "*null" }
	removeplatforms { "*amd64*" }
	removeplatforms { "ps2" } -- for now
	defines { "LCS" }
	debugdir "L:/."

project "vcsview"
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	skeltool("storiesview")
	includedirs { "tools/storiesconv" }
	files { "tools/storiesconv/relocchunk.cpp" }
	files { "tools/storiesconv/rsl.cpp" }
	files { "tools/storiesconv/rslconv.cpp" }
	files { "tools/storiesconv/hash.cpp" }
	includedirs { Zlibdir }
	libdirs { Zlibdir }
	links { "zlib" }
	removeplatforms { "*null" }
	removeplatforms { "*amd64*" }
	removeplatforms { "ps2" } -- for now
	defines { "VCS" }
	debugdir "V:/."

project "euryopa"
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	skeltool("euryopa")
	includedirs { "tools/euryopa" }
	files { "tools/euryopa/minilzo/minilzo.c" }
	removeplatforms { "*null" }
	removeplatforms { "ps2" }

project "gtaclumpview"
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	skeltool("gtaclumpview")
	includedirs { "tools/gtaclumpview" }
	removeplatforms { "*null" }
	removeplatforms { "ps2" } -- for now

project "convifp"
	tool("convifp")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

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
	removelinks { "librwgta" }
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	removeplatforms { "*null" }
	files { path.join("tools/IIItest/*") }
	debugdir "C:/Users/aap/games/gta3"

project "radaredit"
	tool("radaredit")
	kind "ConsoleApp"

project "txdbuild"
	tool("txdbuild")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

