
	BindKey(ALFC_KEY_ALT + string.byte("A"), "About", [[About()]])
	BindKey(ALFC_KEY_F3, "Close Viewer", [[QuitViewer()]])
	BindKey(ALFC_KEY_ALT + string.byte("X"), "Quit", [[:q]])

	BindKey(ALFC_KEY_F1, "Help", [[ShowHelp("$HOME/.alfc/help.hlp", "Main")]])
	BindKey(ALFC_KEY_F2, "Menu", [[Menu()]])

	CreateMenu(ALFC_KEY_ALT + string.byte("F"), "File", {
				{ key = string.byte('c'), name = "Close Viewer", code = [[__CloseViewer()]] },
				{ key = string.byte('x'), name = "Exit", code = [[__QuitApp()]]},
				{ key = string.byte('a'), name = "About", code = [[About()]] },
				{ key = ALFC_KEY_F1, name = "Help", code = [[ViewFile("help.txt")]] }
			})

	CreateMenu(ALFC_KEY_ALT + string.byte("E"), "Edit", {
				{ key = string.byte('h'), name = "Cut", code = [[__Cut()]]},
				{ key = string.byte('x'), name = "Copy", code = [[__Copy()]]},
				{ key = string.byte('a'), name = "Paste", code = [[__Paste()]] }
			})

	CreateMenu(ALFC_KEY_ALT + string.byte("S"), "Search", {
				{ key = string.byte('h'), name = "Find", code = [[__Find()]]},
				{ key = string.byte('x'), name = "Find Next", code = [[__FindNext()]]},
				{ key = string.byte('a'), name = "Replace", code = [[__Replace()]] },
				{ key = string.byte('a'), name = "Goto Line", code = [[__Line()]] }
			})

