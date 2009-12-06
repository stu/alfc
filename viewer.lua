if _G["VIEWER_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Viewer then

	-- globals
	cmds = {}
	bookmarks = {}
	lang = {}


	function CreateMenu(key, name, list)
		local k,v, idx
		idx = AddMenu(key, name)
		for k,v in pairs(list) do
			AddMenuItem(idx, v.key, v.name, v.code)
		end
	end

	function AddCommand(k, d, f)
		cmds[k] = {}
		cmds[k].desc = d
		cmds[k].func = f
	end

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

		if lang == nil then return end

		for k,v in pairs(lang) do
			if v.extensions ~= nil then
				for kk, vv in pairs(v.extensions) do
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
			if string.find(v.name, "^(view_).*([.]lua)$") ~= nil then
				IncludeFile(string.gsub(v.path .. pathsep .. v.name, "\\", "/"))
			end
		end
		lstPlugins = nil
	end

	-- This is an internal function that parses the internal
	-- command line
	-- eg: it turns ":q" into QuitApp etc..
	function __CloseViewer()
		QuitViewer()
	end

	function __QuitApp()
		SetQuitAppFlag(1)
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

	function __ToggleHexMode(command)
		if GetViewMode() == viewModeText then
			SetViewMode(eView_Hext)
		else
			SetViewMode(eView_Text)
		end
	end


	function CLIParse(command)
		local cmd

		cmd = command .. " "

		for k,v in pairs(cmds) do
			if string.sub(cmd, 1, #k) == k then
				v.func(string.sub(cmd, #k, #cmd))
				break
			end
		end
	end

	AddCommand(":q ", "Quit", __QuitApp)
	AddCommand(":t ", "Set tabs", __Tabs)
	AddCommand(":g ", "Go to line or bookmark", __Jump)
	AddCommand(":b ", "Bookmark line", __Bookmark)

	IncludeFile("$HOME/.alfc/viewer_languages.lua")
	IncludeFile("$HOME/.alfc/viewer_menu.lua")

	LoadPlugins()

	OnLoad()


	-- initialise bootstrap and protect code from being called more than once
	-- which should not happen anyway but lets define functions and protect...
	_G["VIEWER_BOOTSTRAP"] = 1

end -- _G["VIEWER_BOOTSTRAP"]
