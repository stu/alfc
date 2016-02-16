local function BuildMenu(win)
	local w1, w2, ws
	local x = {}

	if win == WINDOW_LEFT then
		w1 = "WINDOW_LEFT"
		w2 = "WINDOW_RIGHT"
		ws = "right"
	else
		w1 = "WINDOW_RIGHT"
		w2 = "WINDOW_LEFT"
		ws = "left"
	end

	x = {
		{ key = string.byte('s'), name = "make Same as " .. ws .. "", code = "local x = GetActivePane(); SetActivePane(" .. w2 .. "); __MakeInactivePaneSame(); SetActivePane(x)"},
		{ key = string.byte('c'), name = "Copy to " .. ws .. "", code = "local x = GetActivePane(); SetActivePane(" .. w1 .. "); __TagCopy(); SetActivePane(x)"},
		{ key = string.byte('d'), name = "Delete", code = "local x = GetActivePane(); SetActivePane(" .. w1 .. "); __TagDelete(); SetActivePane(x)"},
		{ key = string.byte('m'), name = "Move to " .. ws .. "", code = "local x = GetActivePane(); SetActivePane(" .. w1 .. "); __TagMove(); SetActivePane(x)"}
	}

	if SystemType() == "UNIX" then
		x[1 + #x] = { key = string.byte('l'), name = "symLink to " .. ws .. "", code = "local x = GetActivePane(); SetActivePane(" .. w1 .. "); __TagSymlink(); SetActivePane(x)"}
	end

	return x
end

	if SystemType() == "UNIX" then
		BindKey(ALFC_KEY_F1, "Help", [[ShowHelp("/usr/local/share/alfc/help.hlp", "Main")]])
	else
		BindKey(ALFC_KEY_F1, "Help", [[ShowHelp("$HOME/.alfc/help.hlp", "Main")]])
	end


	BindKey(ALFC_KEY_F2, "Menu", [[Menu()]])
	BindKey(ALFC_KEY_F3, "View", [[ViewFile(GetHighlightedFilename())]])
	BindKey(ALFC_KEY_F4, "Edit", [[EditFile(GetHighlightedFilename())]])
	BindKey(ALFC_KEY_F5, "Copy", [[:tc]])
	BindKey(ALFC_KEY_F6, "Delete", [[:td]])
	--BindKey(ALFC_KEY_F7, "Create Dir", [[:md]])
	BindKey(ALFC_KEY_F7, "Create Dir", [[:exec local x
	x = OneLineDialogue("Create Directory", "dir", 32)
	if #trim(x) == 0 then
		return
	end
	if( SetCurrentWorkingDirectory(x) == -1) then
		CreateDirectory(trim(x))
		SetCurrentWorkingDirectory(".")
	end]])
	BindKey(ALFC_KEY_F8, "Move", [[:tm]])

	BindKey(ALFC_KEY_ALT + string.byte("X"), "Quit", [[:q]])

	BindKey(ALFC_KEY_F12, "Tag", [[TagHighlightedFile()]])

	BindKey(ALFC_KEY_ALT + string.byte("H"), "Toggle Hidden", [[ToggleHidden()]])

	-- extra's for the ofm1999 standard
	BindKey(ALFC_KEY_CTRL + string.byte("R"), "Refresh", [[:cd]])
	BindKey(ALFC_KEY_CTRL + string.byte("U"), "Swap", [[:swap]])
	-- add in a same quick function
	BindKey(ALFC_KEY_CTRL + string.byte("S"), "Same", [[:s]])

	-- Bind ALT-C, ALT-D, ALT-M for copy/delete/move
	BindKey(ALFC_KEY_ALT + string.byte("C"), "Copy Tagged", [[:tc]])
	BindKey(ALFC_KEY_ALT + string.byte("D"), "Del Tagged", [[:td]])
	BindKey(ALFC_KEY_ALT + string.byte("M"), "Move Tagged", [[:tm]])
	if SystemType() == "UNIX" then
		BindKey(ALFC_KEY_ALT + string.byte("L"), "Symlink Tagged", [[:sym]])
	end

	BindKey(string.byte("{"), "HBck", [[HistoryUp()]])
	BindKey(string.byte("}"), "HFwd", [[HistoryDown()]])


	-- ALT-H for home directory
	BindKey(ALFC_KEY_ALT + string.byte("H"), "Home", [[SetCurrentWorkingDirectory("$HOME")]])


	-- ALT-A for About.. some reason my linux is not passing alt-a back to the system
	-- so we dont get here :(
	BindKey(ALFC_KEY_ALT + string.byte("A"), "About", [[About()]])
	CreateMenu(ALFC_KEY_ALT + string.byte("1"), "Left Panel", BuildMenu(WINDOW_LEFT))

	CreateMenu(ALFC_KEY_ALT + string.byte("F"), "File", {
				{ key = string.byte('h'), name = "Hidden", code = [[if GetOption("options", "show_hidden") == "false" then SetOption("options", "show_hidden", "true") else SetOption("options", "show_hidden", "false") end SetCurrentWorkingDirectory(GetCurrentWorkingDirectory())]]},
				{ key = string.byte('x'), name = "Exit", code = [[__QuitApp()]]},
				{ key = string.byte('a'), name = "About", code = [[About()]] },
				{ key = ALFC_KEY_F1, name = "Help", code = [[if SystemType() == "UNIX" then
		ShowHelp("/usr/local/share/alfc/help.hlp", "Main")
	else
		ShowHelp("$HOME/.alfc/help.hlp", "Main")
	end]] },
			})

	CreateMenu(ALFC_KEY_ALT + string.byte("2"), "Right Panel", BuildMenu(WINDOW_RIGHT))
