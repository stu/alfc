debug_msg("\n** Welcome to ALFC v" .. GetVersionString() .. " **\n")

-- check for root and switch frame colours to red!
if GetUserID() == 0 or GetUserGroup() == 0 then
	debug_msg("\n\n** WARNING ** Running as root\n\n")
	SetColour(title_foreground, "yellow")
	SetColour(title_background, "red")
end

