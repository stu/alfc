if _G["VIEWER_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Viewer then

	cmds = {}

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

		cmds = {}

	-- This is an internal function that parses the internal
	-- command line
	-- eg: it turns ":q" into QuitApp etc..
	function __QuitApp()
		QuitViewer()
	end

	function __Tabs(command)
		SetTabSize(tonumber(command))
	end

	function CLIParse(command)
		local cmd
		local cmds

		cmd = command .. " "

		cmds[":q "] = __QuitApp
		cmds[":t "] = __Tabs

		for k,v in pairs(cmds) do
			if string.sub(cmd, 1, #k) == k then
				v(string.sub(cmd, #k, #cmd))
				break
			end
		end
	end


	LoadPlugins()

	BindKey(ALFC_KEY_ALT + string.byte("A"), "About", [[About()]])
	BindKey(ALFC_KEY_F10, "Quit", [[:q]])


	-- initialise bootstrap and protect code from being called more than once
	-- which should not happen anyway but lets define functions and protect...
	_G["VIEWER_BOOTSTRAP"] = 1

end -- _G["BOOTSTRAP"]
