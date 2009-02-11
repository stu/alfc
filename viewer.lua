if _G["VIEWER_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Viewer then

-- This is an internal function that parses the internal
-- command line
-- eg: it turns ":q" into QuitApp etc..
local function __QuitApp()
	QuitViewer()
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
