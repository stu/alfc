if _G["LIST_BOOTSTRAP"] ~= 1 and GetMode() == eMode_VB_List then

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

	-- called at the end of this script
	function OnLoad()
		-- nothing to see here... yet.
	end

	function LoadPlugins()
		local k, v, lstPlugins

		lstPlugins = GetFileListFromPath("$HOME/.alfc");
		table.sort(lstPlugins, function(a,b) return a.name<b.name end)

		for k, v in pairs(lstPlugins) do
			-- match 'core_' * '.lua'
			if string.find(v.name, "^(list_).*([.]lua)$") ~= nil then
				if v.name ~= "list_menu.lua" then
					IncludeFile(string.gsub(v.path .. pathsep .. v.name, "\\", "/"))
				end
			end
		end
		lstPlugins = nil
	end

	-- This is an internal function that parses the internal
	-- command line
	-- eg: it turns ":q" into QuitApp etc..

	function __QuitApp()
		SetQuitAppFlag(1)
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

	function __ViewFile(command)
		ViewFile(GetHighlightedFilename())
	end


	AddCommand(":q ", "Quit", __QuitApp)

	IncludeFile("$HOME/.alfc/list_menu.lua")

	LoadPlugins()
	OnLoad()

	-- initialise bootstrap and protect code from being called more than once
	-- which should not happen anyway but lets define functions and protect...
	_G["LIST_BOOTSTRAP"] = 1

end -- _G["LIST_BOOTSTRAP"]
