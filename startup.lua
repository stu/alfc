debug_msg("\n** Welcome to ALFC v" .. VersionString() .. " **")
debug_msg("Using driver : " .. DriverName() .. " on system type " .. SystemType() .. "\n")

-- check for root and switch frame colours to red!
if GetUserID() == 0 or GetUserGroup() == 0 then
	local s;

	-- unnecessarily big ass warning :)
	s = " WARNING : Running as root "
	s = "\n" .. string.rep("*", (GetScreenWidth() - (#s +1))/2) .. s ..  string.rep("*", (GetScreenWidth() - (#s +1))/2) .. "\n"
	debug_msg(s)
	debug_msg("\n  ")
	SetColour(title_foreground, "yellow")
	SetColour(title_background, "red")
	SetColour(highlight_foreground, "white")
	SetColour(highlight_background, "blue")
end

if GetMode() == eMode_Directory then
	-- display hints first time loaded
	if GetOption("options", "hints") == "true" then
		local h

		IncludeFile("$HOME/.alfc/hints.lua")

		-- convert to number
		h = "0" .. GetOption("options", "current_hint")
		h = 0 + h

		if h < 1 or h > #hints then
			h = 1
		end

		SetOption("options","current_hint", "" .. (1 + h))

		msgbox(hints[h])
	end
end
