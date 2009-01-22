--[[

	Global Lua Functions, extra commands that can be called by ALFC

	Contains the global functions (separate) for
		Directory Mode,
		Viewer
]]

if _G["DIR_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Directory then

	-- merge two tables together
	local function merge(t, u)
		local r = {}
		local k, v
		for k, v in ipairs(t) do
			r[1+#r] = v
		end
		for k, v in ipairs(u) do
			r[1+#r] = v
		end
		return r
	end


	-- This is an internal function that parses the internal
	-- command line
	-- eg: it turns ":q" into QuitApp etc..
	local function __QuitApp()
		SetQuitAppFlag(1)
	end

	local function __FilterAdd(command)
		local cmd

		cmd = trim(command)
		AddFilter(cmd)
	end

	local function __Filter(command)
		local cmd

		cmd = trim(command)
		SetFilter(cmd)
	end

	local function __GlobAdd(command)
		local cmd

		cmd = trim(command)
		AddGlob(cmd)
	end

	local function __Glob(command)
		local cmd

		cmd = trim(command)
		SetGlob(cmd)
	end

	local function __MakeInactivePaneSame(command)
		local npath

		-- Get path from active window
		npath = GetCurrentWorkingDirectory()
		-- switch to inactive window and set new directory
		SwitchPanes()
		SetCurrentWorkingDirectory(npath)

		-- switch back to original working pane
		SwitchPanes()
	end


	-- this wont work when you want a file and directories are first listed....
	local function __Jump(command)
		local j
		local in_dir

		local best_match_length
		local best_match

		local cmd

		fl = GetFileList()

		best_match_length = 0
		best_match = -1

		cmd = trim(command)

		for k, v in ipairs(fl) do
			for j=1, #cmd do

				if string.sub(v.name, j, j) == string.sub(cmd, j, j) then
					if j > best_match_length then
						best_match_length = j
						best_match = k
					end
				else
					break
				end

			end
		end

		if best_match ~= -1 then
			SetHighlightedFile(best_match - 1)
		end

	end

	local function __ChangeDir(command)
		local cmd = command

		-- try to deal with spaces
		if( SetCurrentWorkingDirectory(command) == -1) then
			SetCurrentWorkingDirectory(trim(command))
		end
	end

	local function __SortOrder(command)
		if trim(command) == "na" then
			SetOption("options", "sort_order", "name_asc")
		elseif trim(command) == "nd" then
			SetOption("options", "sort_order", "name_desc")
		elseif trim(command) == "sa" then
			SetOption("options", "sort_order", "size_asc")
		elseif trim(command) == "sd" then
			SetOption("options", "sort_order", "size_desc")
		end

		-- re-sort..
		SortFileList()
		RedrawWindow()
	end

	function CLIParse(command)
		local cmd
		local cmds

		cmd = command .. " "

		cmds = {}
		cmds[":q "] = __QuitApp
		cmds[":f "] = __Filter
		cmds[":f+ "] = __FilterAdd
		cmds[":g "] = __Glob
		cmds[":g+ "] = __GlobAdd
		cmds[":s "] = __MakeInactivePaneSame
		cmds[":j "] = __Jump
		cmds[":c "] = __ChangeDir
		cmds[":so "] = __SortOrder

		for k,v in pairs(cmds) do
			if string.sub(cmd, 1, #k) == k then
				v(string.sub(cmd, #k, #cmd))
				break
			end
		end
	end

	-- This function takes our list of files and sorts them
	-- how the user wants them to be sorted...
	function SortFileList()
		local lstD = {}
		local lstF = {}
		local lstSF = {}
		local lstSD = {}

		local opt_dirs_first;
		local opt_sort_order;
		local sortfunc

		opt_dirs_first = GetOption("options", "directories_first")
		opt_sort_order = GetOption("options", "sort_order")

		lstF = GetFileList()

		for k,v in ipairs(lstF) do
			if opt_dirs_first == "true" then
				if v.directory == 1 then
					lstSD[1 + #lstSD] = {}
					lstSD[#lstSD].key = k
					lstSD[#lstSD].data = v
				else
					lstSF[1 + #lstSF] = {}
					lstSF[#lstSF].key = k
					lstSF[#lstSF].data = v
				end
			else
				lstSF[1 + #lstSF] = {}
				lstSF[#lstSF].key = k
				lstSF[#lstSF].data = v
			end
		end


		if opt_sort_order == "name_desc" then
			sortfunc = function(a,b) return a.data.name>b.data.name end
		elseif opt_sort_order == "name_asc" then
			sortfunc = function(a,b) return a.data.name<b.data.name end
		elseif opt_sort_order == "size_desc" then
			sortfunc = function(a,b) return a.data.size>b.data.size end
		elseif opt_sort_order == "size_asc" then
			sortfunc = function(a,b) return a.data.size<b.data.size end
		else
			-- default use name ascending...
			sortfunc = function(a,b) return a.data.name<b.data.name end
		end

		if opt_dirs_first == "true" then
			-- sort by name for dirs always
			table.sort(lstSD, function(a,b) return a.data.name<b.data.name end)

			-- now sort files
			table.sort(lstSF, sortfunc)

			lstSF = merge(lstSD, lstSF)
			lstSD = nil
		else
			-- sort by request
			table.sort(lstSF, sortfunc)
		end

		ClearSortList()
		for k,v in ipairs(lstSF) do
			--debug_msg(v.data.name)
			AddToSortList(v.data.name)
		end

	end

	--debug_msg("Global Lua Functions bootstrapped")
	BindKey(ALFC_KEY_F01, "View", [[ViewFile(GetHighlightedFilename())]])
	BindKey(ALFC_KEY_F02, "Same", [[:s]])
	BindKey(ALFC_KEY_F10, "Quit", [[:q]])

	-- Sometimes I want a quick view for code files
	BindKey(ALFC_KEY_F11, "CodeOnly", [[SetFilter("\\.[ch]$"); SetGlob("*.lua")]])
	BindKey(ALFC_KEY_F12, "Tag", [[TagHighlightedFile()]])

	_G["DIR_BOOTSTRAP"] = 1
end -- _G["BOOTSTRAP"]





if _G["VIEWER_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Viewer then

-- This is an internal function that parses the internal
-- command line
-- eg: it turns ":q" into QuitApp etc..
local function __QuitApp()
	SetQuitAppFlag(1)
end

local function __Tabs(command)
	SetTabSize(tonumber(command))
end

function CLIParse(command)
	local cmd
	local cmds

	cmd = command .. " "

	cmds = {}
	cmds[":q "] = __QuitApp
	cmds[":t "] = __Tabs

	for k,v in pairs(cmds) do
		if string.sub(cmd, 1, #k) == k then
			v(string.sub(cmd, #k, #cmd))
			break
		end
	end
end

	BindKey(ALFC_KEY_F10, "Quit", [[:q]])

	-- initialise bootstrap and protect code from being called more than once
	-- which should not happen anyway but lets define functions and protect...
	_G["VIEWER_BOOTSTRAP"] = 1

end -- _G["BOOTSTRAP"]


