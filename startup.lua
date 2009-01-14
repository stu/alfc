debug_msg("\n** Welcome to ALFC v" .. GetVersionString() .. " **\n")

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

