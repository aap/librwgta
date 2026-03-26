Librw = os.getenv("LIBRW")
Zlib = os.getenv("ZLIBDIR")
Lua = os.getenv("LUADIR")

if not Librw then
    error("Please set the LIBRW environment variable to your librw folder path.")
end

newoption {
	trigger		= "gfxlib",
	value       = "LIBRARY",
	description = "Choose a particular development library",
	default		= "glfw",
	allowed		= {
		{ "glfw",	"GLFW" },
		{ "sdl2",	"SDL2" },
	},
}

newoption {
	trigger     = "glfwdir64",
	value       = "PATH",
	description = "Directory of glfw",
	default     = "../glfw-3.3.4.bin.WIN64",
}

newoption {
	trigger     = "glfwdir32",
	value       = "PATH",
	description = "Directory of glfw",
	default     = "../glfw-3.3.4.bin.WIN32",
}

newoption {
	trigger     = "sdl2dir",
	value       = "PATH",
	description = "Directory of sdl2",
	default     = "../SDL2-2.0.14",
}

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
			"linux-amd64-null", "linux-amd64-gl3",
			"linux-arm-null", "linux-arm-gl3" }
	-- TODO: ps2
		if _OPTIONS["gfxlib"] == "sdl2" then
			includedirs { "/usr/include/SDL2" }
		end
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
		if _OPTIONS["gfxlib"] == "glfw" then
			defines { "LIBRW_GLFW" }
		else
			defines { "LIBRW_SDL2" }
		end
	filter { "platforms:*d3d9" }
		defines { "RW_D3D9" }
	filter { "platforms:*ps2" }
		defines { "RW_PS2" }
		toolset "gcc"
		gccprefix 'ee-'
		buildoptions { "-nostdlib", "-fno-common" }
		includedirs { "$(PS2SDK)/ee/include", "$(PS2SDK)/common/include" }
		optimize "Off"

	filter { "platforms:*amd64*" }
		architecture "x86_64"
	filter { "platforms:*x86*" }
		architecture "x86"
	filter { "platforms:*arm*" }
		architecture "ARM"

	filter { "platforms:win*" }
		system "windows"
	filter { "platforms:linux*" }
		system "linux"

	filter { "platforms:win*gl3" }
		defines { "GLEW_STATIC" }
		includedirs { path.join(_OPTIONS["glewdir"], "include") }
		includedirs { path.join(_OPTIONS["sdl2dir"], "include") }
	filter { "platforms:win-x86-gl3" }
		includedirs { path.join(_OPTIONS["glfwdir32"], "include") }
	filter { "platforms:win-amd64-gl3" }
		includedirs { path.join(_OPTIONS["glfwdir64"], "include") }

	filter "action:vs*"
		buildoptions { "/wd4996", "/wd4244" }
		
	filter { "platforms:win*gl3", "action:not vs*" }
		if _OPTIONS["gfxlib"] == "sdl2" then
			includedirs { "/mingw/include/SDL2" } -- TODO: Detect this properly
		end

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
		links { "GL" }
		if _OPTIONS["gfxlib"] == "glfw" then
			links { "glfw" }
		else
			links { "SDL2" }
		end
	filter { "platforms:win-amd64-gl3" }
		libdirs { path.join(_OPTIONS["glfwdir64"], "lib-vc2015") }
		libdirs { path.join(_OPTIONS["sdl2dir"], "lib/x64") }
	filter { "platforms:win-x86-gl3" }
		libdirs { path.join(_OPTIONS["glfwdir32"], "lib-vc2015") }
		libdirs { path.join(_OPTIONS["sdl2dir"], "lib/x86") }
	filter { "platforms:win*gl3" }
		links { "opengl32" }
		if _OPTIONS["gfxlib"] == "glfw" then
			links { "glfw3" }
		else
			links { "SDL2" }
		end
	filter { "platforms:*d3d9" }
		links { "gdi32", "d3d9" }
	filter { "platforms:*d3d9", "action:vs*" }
		links { "Xinput9_1_0" }
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

project "selanna"
	tool("selanna")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

	if not Lua then
		includedirs { Lua }
		links { "lua5.4" }
	else
		print("Warning: LUADIR not set; selenna may fail to compile.")
	end

	local env = os.getenv("SELANNA_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "convdff"
	tool("convdff")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

	local env = os.getenv("CONVDFF_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "convtxd"
	tool("convtxd")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

	local env = os.getenv("CONVTXD_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "lcsconv"
	tool("storiesconv")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }
	defines { "LCS" }

	local env = os.getenv("LCSCONV_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "vcsconv"
	tool("storiesconv")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }
	defines { "VCS" }
	debugdir "%VCSCONV_DEBUGDIR%"

	local env = os.getenv("VCSCONV_DEBUGDIR")
	if env then
		debugdir(env)
	end

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

	if not zlib then
		includedirs { Zlib }
		libdirs { Zlib }
		links { "zlib" }
	else
		print("Warning: ZLIBDIR not set; lcsview may fail to compile.")
	end

	removeplatforms { "*null" }
	removeplatforms { "*amd64*" }
	removeplatforms { "ps2" } -- for now
	defines { "LCS" }

	local env = os.getenv("LCSVIEW_DEBUGDIR")
	if env then
		debugdir(env)
	end

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

	if not zlib then
		includedirs { Zlib }
		libdirs { Zlib }
		links { "zlib" }
	else
		print("Warning: ZLIBDIR not set; vcsview may fail to compile.")
	end

	removeplatforms { "*null" }
	removeplatforms { "*amd64*" }
	removeplatforms { "ps2" } -- for now
	defines { "VCS" }

	local env = os.getenv("VCSVIEW_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "euryopa"
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	skeltool("euryopa")
	includedirs { "tools/euryopa" }
	files { "tools/euryopa/minilzo/minilzo.c" }
	removeplatforms { "*null" }
	removeplatforms { "ps2" }

	local env = os.getenv("EURYOPA_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "gtaclumpview"
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	skeltool("gtaclumpview")
	includedirs { "tools/gtaclumpview" }
	removeplatforms { "*null" }
	removeplatforms { "ps2" } -- for now

	local env = os.getenv("GTACLUMPVIEW_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "convifp"
	tool("convifp")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

	local env = os.getenv("CONVIFP_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "coltest"
	tool("coltest")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

	local env = os.getenv("COLTEST_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "d3d9test"
	tool("d3d9test")
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	removeplatforms { "*gl3", "*null" }
	links { "winmm" }

	local env = os.getenv("D3D9TEST_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "IIItest"
	tool("IIItest")
	removelinks { "librwgta" }
	kind "WindowedApp"
	characterset ("MBCS")
	flags { "WinMain" }
	removeplatforms { "*null" }
	files { path.join("tools/IIItest/*") }

	local env = os.getenv("IIITEST_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "radaredit"
	tool("radaredit")
	kind "ConsoleApp"

	local env = os.getenv("RADAREDIT_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "txdbuild"
	tool("txdbuild")
	kind "ConsoleApp"
	removeplatforms { "*gl3", "*d3d9" }

	local env = os.getenv("TXDBUILD_DEBUGDIR")
	if env then
		debugdir(env)
	end

project "convcdimage"
	tool("convcdimage")
	kind "ConsoleApp"
	files { "tools/euryopa/minilzo/minilzo.c" }
	removeplatforms { "*gl3", "*d3d9" }

	local env = os.getenv("CONVCDIMAGE_DEBUGDIR")
	if env then
		debugdir(env)
	end

