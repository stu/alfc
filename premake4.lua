solution "alfm"
	xflag = false

	configurations { "Debug", "Release" }
		configuration "Debug"
			targetdir "debug"
			defines { "DEBUG" }
			flags { "Symbols" }

		configuration "Release"
			targetdir "release"
			defines { "NDEBUG" }
			flags { "Optimize" }


	project "bin2c"
		kind "ConsoleApp"
		language "C"
		files "bin2c.c"
	  	objdir "obj"
        targetdir "tools"

   -- relies on bin2c being built
	project "alfc"
		kind "ConsoleApp"
		language "C"
		targetdir "."
		files { "*.h", "*.c", "lua_common_api.c", "lua_common_api.h", "lua_helper.c","lua_helper.h", "lua_helper_viewer.c", "lau_helper_viewer.h", "version.c","version.h"  }
		excludes { "bin2c.c", "gcomp.c", "guide.c", "guidedump.c", "gdump.c", "ncurses_guide.c", "guide_guide.c" }
	 	includedirs { "." }
		objdir "obj"
		prebuildcommands {
		"ruby " .. solution().basedir .. "/tools/buildversion.rb",
		"ruby " .. solution().basedir .. "/tools/helpcompiler.rb docs/help.txt docs/help.hlp",
		"tools/bin2c global.lua include_global_lua > defaults.h",
		"tools/bin2c viewer.lua include_viewer_lua >> defaults.h",
		"tools/bin2c startup.lua include_startup_lua >> defaults.h",
		"tools/bin2c shutdown.lua include_shutdown_lua >> defaults.h",
		"tools/bin2c options.ini include_options_ini >> defaults.h",
		"tools/bin2c core_extract.lua core_extract_lua >> defaults.h",
		"tools/bin2c core_hash.lua core_hash_lua >> defaults.h",
		"tools/bin2c viewer_languages.lua viewer_languages_lua >> defaults.h",
		"tools/bin2c filemanager_menu.lua filemanager_menu_lua >> defaults.h",
		"tools/bin2c hints.lua hints_lua >> defaults.h",
		"tools/bin2c viewer_menu.lua viewer_menu_lua >> defaults.h",
		"tools/bin2c docs/help.hlp help_hlp >> defaults.h",
		--always build gui fonts
		"echo \"#ifdef DRV_GUI\" > gui_fonts.h",
		"tools/bin2c font.rlf gui_data_font >> gui_fonts.h",
		"tools/bin2c font_small.rlf gui_data_font_small >> gui_fonts.h",
		"echo \"#endif\" >> gui_fonts.h",

		-- gen lua files
		"lua " .. solution().basedir .. "/tools/lua_helper.lua",
				 }

		libluaxx = "lua5"
		liblua = os.findlib("lua")
		if liblua == nil then
			libluaxx = "lua5"
			liblua = os.findlib("lua5")
			if liblua == nil then
				liblua = os.findlib("lua5.1")
				libluaxx = "lua5.1"
			end
		end

		local inclua = os.pathsearch("lualib.h", "/usr/include/lua5.1;/usr/include/lua5;/usr/include/lua;/usr/include;/usr/local/include;/usr/sfw/include;/mingw/include;/mingw/include/lua;/opt/lua/include;/opt/lua5.1/include")
		includedirs { inclua }
		libdirs { liblua }

		configuration "linux"
			buildoptions { "-D__USE_LARGEFILE64=1","-D_FILE_OFFSET_BITS=64","-DBUILD_UNIXLIKE","-D_FORTIFY_SOURCE=0"}
			if _ARGS[1] == "ncurses" then
				defines { "DRV_NCURSES","BUILD_UNIXLIKE"}
				links { "m", "ncurses", libluaxx, "dl", "z" }
				files { "portability_unix.c", "ncurses_main.c", "ncurses_interface.c" }
				excludes { "gui_main.c", "guicore.c", "portability_win32.c" }
				xflag = true
			elseif _ARGS[1] == "x11" then
				defines { "DRV_GUI","BUILD_UNIXLIKE"}
				links { "m", libluaxx, "X11", "dl", "z" }
				files { "portability_unix.c", "gui_main.c", "gui_interface.c", "guicore.c" }
				excludes { "portability_win32.c", "gui_guide.c", "ncurses_main.c", "ncurses_guide.c" }
				xflag = true
			end

		configuration "windows"
			if _ARGS[1] == "pdcurses" then
				defines { "DRV_NCURSES", "BUILD_WIN32"}
				links { "pdcurses", libluaxx, "regex", "iberty", "kernel32", "z" }
				files { "portability_win32.c", "ncurses_main.c", "ncurses_interface.c" }
				excludes { "gui_main.c", "guicore.c", "portability_unix.c" }
				xflag = true
			elseif _ARGS[1] == "win32" then
				defines { "DRV_GUI", "BUILD_WIN32"}
				links { libluaxx,"regex", "iberty", "kernel32", "z", "user32", "gdi32" }
				files { "portability_win32.c", "gui_main.c", "gui_interface.c", "guicore.c" }
				excludes { "portability_unix.c", "gui_guide.c", "ncurses_main.c", "ncurses_guide.c" }
				xflag = true
			end

	-- guide relies on alfm being built (uses some of its files)
	project "guide"
		kind "ConsoleApp"
		language "C"
		files { "guide.c", "guideload.c", "guidedisplay.c", "dlist.c", "version.c", "logwrite.c" }
	  	objdir "obj"
	  	includedirs { "." }
	  	targetdir "."

		libluaxx = "lua5"
		liblua = os.findlib("lua")
		if liblua == nil then
			libluaxx = "lua5"
			liblua = os.findlib("lua5")
			if liblua == nil then
				liblua = os.findlib("lua5.1")
				libluaxx = "lua5.1"
			end
		end

		local inclua = os.pathsearch("lualib.h", "/usr/include/lua5.1;/usr/include/lua5;/usr/include/lua;/usr/include;/usr/local/include;/usr/sfw/include;/mingw/include;/mingw/include/lua;/opt/lua/include;/opt/lua5.1/include")
			includedirs { inclua }
			libdirs { liblua }

		configuration "linux"
			buildoptions { "-D__USE_LARGEFILE64=1","-D_FILE_OFFSET_BITS=64","-DBUILD_UNIXLIKE","-D_FORTIFY_SOURCE=0"}
			if _ARGS[1] == "ncurses" then
				defines { "DRV_NCURSES","BUILD_UNIXLIKE"}
				links { "m", "ncurses", libluaxx, "dl", "z" }
				files { "portability_unix.c", "ncurses_guide.c", "ncurses_interface.c" }
				excludes { "gui_main.c", "guicore.c", "portability_win32.c", "gui_guide.c", "gui_interface.c" }
				xflag = true
			elseif _ARGS[1] == "x11" then
				defines { "DRV_GUI","BUILD_UNIXLIKE"}
				links { "m", libluaxx, "X11", "dl", "z" }
				files { "portability_unix.c", "gui_guide.c", "gui_interface.c", "guicore.c" }
				excludes { "portability_win32.c", "ncurses_guide.c" }
				xflag = true
			end

		configuration "windows"
			if _ARGS[1] == "win32" then
				defines { "DRV_GUI", "BUILD_WIN32"}
				links { libluaxx, "regex", "iberty", "kernel32","user32", "gdi32","z" }
				files { "portability_win32.c", "gui_main.c", "gui_interface.c", "guicore.c" }
				xflag = true
			elseif _ARGS[1] == "pdcurses" then
				defines { "DRV_NCURSES", "BUILD_WIN32"}
				links { "pdcurses", libluaxx, "regex", "iberty", "kernel32", "z" }
				files { "portability_win32.c", "ncurses_main.c", "ncurses_interface.c" }
				xflag = true
			end

	project "gdump"
		kind "ConsoleApp"
		language "C"
		files { "guidedump.c", "guideload.c", "dlist.c", "logwrite.c" }
	  	objdir "obj"
	  	links { "z" }
	  	targetdir "."

if _ACTION ~= "clean" then
	if xflag == false then
		if os.get() == "linux" then
			drv_string = "ncurses or x11"
		elseif os.get() == "windows" then
			drv_string = "pdcurses or win32"
		else
			drv_string = " UNKNOWN OS " .. os.get()
		end
		error("Select system (gmake, vs2005, etc) and appropriate driver " .. drv_string .. "\neg: premake4 gmake x11", 0)
	end
end

if _ACTION == "clean" then
	os.rmdir "obj"
	os.rmdir "debug"
	os.rmdir "release"
end
