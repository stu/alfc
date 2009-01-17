--[[

	Global Lua Functions, extra commands that can be called by ALFC

]]

if _G["BOOTSTRAP"] ~= 1 then

-- This is called when this script is booted
function GlobalLuaFuncs()
	debug_msg("Global Lua Functions bootstrapped")
end

-- This is an internal function that parses the internal
-- command line
-- eg: it turns ":q" into QuitApp etc..
local function __QuitApp()
	SetQuitAppFlag(1)
end

local function __Filter(command)
	debug_msg("filter " .. command)
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
local function __FindFirst(command)
	local fn
	local fl

	fl = GetFileList()

	for k,v in pairs(fl) do
		debug_msg("name = " .. v.name .. " size = " .. v.size .. " directory = " .. v.directory)
	end

end

function CLIParse(command)
	local cmd
	local cmds

	cmd = command .. " "

	cmds = {}
	cmds[":q "] = __QuitApp
	cmds[":f "] = __Filter
	cmds[":s "] = __MakeInactivePaneSame
	cmds[":ff "] = __FindFirst

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

	opt_dirs_fist = GetOption("options", "directories_first")
	opt_sort_order = GetOption("options", "sort_order")

end

	-- initialise bootstrap and protect code from being called more than once
	-- which should not happen anyway but lets define functions and protect...
	_G["BOOTSTRAP"] = 1
	GlobalLuaFuncs()
end -- _G["BOOTSTRAP"]
