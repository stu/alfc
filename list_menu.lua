
	if SystemType() == "UNIX" then
		BindKey(ALFC_KEY_F1, "Help", [[ShowHelp("/usr/local/share/alfc/help.hlp", "List")]])
    else
		BindKey(ALFC_KEY_F1, "Help", [[ShowHelp("$HOME/.alfc/help.hlp", "List")]])
    end

	BindKey(ALFC_KEY_ALT + string.byte("A"), "About", [[About()]])
	BindKey(ALFC_KEY_ALT + string.byte("X"), "Quit", [[:q]])

	BindKey(ALFC_KEY_F2, "Menu", [[Menu()]])
	BindKey(ALFC_KEY_F3, "View", [[__ViewFile(GetHighlightedFilename())]])


	CreateMenu(ALFC_KEY_ALT + string.byte("F"), "File", {
				{ key = string.byte('x'), name = "Exit", code = [[__QuitApp()]]},
				{ key = string.byte('a'), name = "About", code = [[About()]] },
				{ key = ALFC_KEY_F1, name = "Help", code = [[if SystemType() == "UNIX" then
		ShowHelp("/usr/local/share/alfc/help.hlp", "Main")
    else
		ShowHelp("$HOME/.alfc/help.hlp", "Main")
    end]] }
			})
