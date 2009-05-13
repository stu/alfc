if _G["VIEWER_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Viewer then

	-- globals
	cmds = {}
	bookmarks = {}
	lang = {}

	function findpattern(text, pattern, start)
		local p
		p = string.find(text, pattern, start)
		if p ~= nil then
			return string.sub(text, p, #text)
		else
			return ""
		end
	end


	-- called at the end of this script
	function OnLoad()
		local k,v
		local fn, ext

		fn = ViewedFilename()
		ext = findpattern(fn, "[.]")

		--debug_msg("fn="..fn .. " lang count="..#lang)

		if lang == nil then return end
		if #lang == 0 then return end

		for k,v in pairs(lang) do
			if v.extensions ~= nil then
				for kk, vv in pairs(v.extensions) do
					debug_msg("vv=" .. vv .. "ext="..ext)
					if vv == ext then
						SetTabSize(tonumber(v.tab))
						SetFileType(v.name);
						break
					end
				end
			end
		end
	end

	function LoadPlugins()
		local k, v, lstPlugins

		lstPlugins = GetFileListFromPath("$HOME/.alfc");
		table.sort(lstPlugins, function(a,b) return a.name<b.name end)

		for k, v in pairs(lstPlugins) do
			-- match 'core_' * '.lua'
			if string.find(v.name, "^(viewer_).*([.]lua)$") ~= nil then
				IncludeFile(string.gsub(v.path .. pathsep .. v.name, "\\", "/"))
			end
		end
		lstPlugins = nil
	end

	-- This is an internal function that parses the internal
	-- command line
	-- eg: it turns ":q" into QuitApp etc..
	function __QuitApp()
		QuitViewer()
	end

	function __Tabs(command)
		SetTabSize(tonumber(command))
	end

	function __Jump(command)
		local l

		if #trim(command) == 0 then return end

		l = tonumber(trim(command))
		if l ~= nil and l > 0 then
			GoToLine(l)
		else
			if bookmarks[trim(command)] ~= nil then
				GoToLine(bookmarks[trim(command)])
			end
		end
	end

	function __Bookmark(command)
		if #trim(command) > 0 then
			bookmarks[trim(command)] = GetCurrentLineNumber()
		end
	end

	function CLIParse(command)
		local cmd

		cmd = command .. " "

		for k,v in pairs(cmds) do
			if string.sub(cmd, 1, #k) == k then
				v(string.sub(cmd, #k, #cmd))
				break
			end
		end
	end

	cmds[":q "] = __QuitApp
	cmds[":t "] = __Tabs
	cmds[":g "] = __Jump
	cmds[":b "] = __Bookmark

	LoadPlugins()

	BindKey(ALFC_KEY_ALT + string.byte("A"), "About", [[About()]])
	BindKey(ALFC_KEY_F3, "Quit", [[:q]])


	OnLoad()

	-- initialise bootstrap and protect code from being called more than once
	-- which should not happen anyway but lets define functions and protect...
	_G["VIEWER_BOOTSTRAP"] = 1

end -- _G["VIEWER_BOOTSTRAP"]
