--[[

	Global Lua Functions, extra commands that can be called by ALFC

	Contains the global functions (separate) for
		Directory Mode,
]]

if _G["DIR_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Directory then

	cmds = {}


	function CreateMenu(key, name, list)
		local k,v
		--debug_msg("Create menu " .. name)

		for k,v in pairs(list) do
			--debug_msg("\t" .. v.name)
		end

	end

	function comma_value(amount)
		local formatted = amount
		while true do
			formatted, k = string.gsub(formatted, "^(-?%d+)(%d%d%d)", '%1,%2')
			if (k==0) then
				break
			end
		end
		return formatted
	end


	---============================================================
	-- rounds a number to the nearest decimal places
	--
	function round(val, decimal)
		if (decimal) then
			return math.floor( (val * 10^decimal) + 0.5) / (10^decimal)
		else
			return math.floor(val+0.5)
		end
	end


	--===================================================================
	-- given a numeric value formats output with comma to separate thousands
	-- and rounded to given decimal places
	--
	--
	function format_num(amount, decimal, prefix, neg_prefix)
		local str_amount,  formatted, famount, remain

		decimal = decimal or 2  -- default 2 decimal places
		neg_prefix = neg_prefix or "-" -- default negative sign

		famount = math.abs(round(amount,decimal))
		famount = math.floor(famount)

		remain = round(math.abs(amount) - famount, decimal)

		-- comma to separate the thousands
		formatted = comma_value(famount)

		-- attach the decimal portion
		if (decimal > 0) then
			remain = string.sub(tostring(remain),3)
			formatted = formatted .. "." .. remain ..
			string.rep("0", decimal - string.len(remain))
		end

		-- attach prefix string e.g '$'
		formatted = (prefix or "") .. formatted

		-- if value is negative then format accordingly
		if (amount<0) then
			if (neg_prefix=="()") then
				formatted = "("..formatted ..")"
			else
				formatted = neg_prefix .. formatted
			end
		end

		return formatted
	end


	function size_mash(s)
		local c = { 1024, 1024*1024, 1024*1024*1024, 1024*1024*1024*1024, 1024*1024*1024*1024*1024 }

		if s < c[1] then
			return s .. " bytes"
		elseif s < c[2] then
			return format_num(s/c[1], 2) .. "kb"
		elseif s < c[3] then
			return format_num(s / c[2], 2) .. "mb"
		elseif s < c[4] then
			return format_num(s / c[3], 2) .. "gb"
		else
			return format_num(s / c[5], 2) .. "tb"
		end
	end

	-- merge two tables together
	function merge(t, u)
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
	function __QuitApp()
		SetQuitAppFlag(1)
	end

	function __FilterAdd(command)
		local cmd

		cmd = trim(command)
		AddFilter(cmd)
	end

	function __Filter(command)
		local cmd

		cmd = trim(command)
		SetFilter(cmd)
	end

	function __GlobAdd(command)
		local cmd

		cmd = trim(command)
		AddGlob(cmd)
	end

	function __Glob(command)
		local cmd

		cmd = trim(command)
		SetGlob(cmd)
	end

	function __MakeInactivePaneSame(command)
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
	function __Jump(command)
		local j
		local in_dir

		local best_match_length
		local best_match

		local cmd
		local k, v

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

	function __ChangeDir(command)
		local cmd = command

		-- try to deal with spaces
		if( SetCurrentWorkingDirectory(command) == -1) then
			SetCurrentWorkingDirectory(trim(command))
		end
	end

	function __SortOrder(command)
		if trim(command) == "na" then
			SetOption("options", "sort_order", "name_asc")
		elseif trim(command) == "nd" then
			SetOption("options", "sort_order", "name_desc")
		elseif trim(command) == "sa" then
			SetOption("options", "sort_order", "size_asc")
		elseif trim(command) == "sd" then
			SetOption("options", "sort_order", "size_desc")
		elseif trim(command) == "da" then
			SetOption("options", "sort_order", "date_asc")
		elseif trim(command) == "dd" then
			SetOption("options", "sort_order", "date_desc")
		end

		-- re-sort..
		SortFileList()
		SwitchPanes()
		SortFileList()
		RedrawWindow()
		SwitchPanes()
		RedrawWindow()
	end

	function __TagCopyX(command, buff)
		local lstF
		local lstT
		local k,v, err, errc
		local file_size
		local file_count

		file_size = 0
		file_count = 0

		local path1, path2

		lstF = GetTaggedFileList()
		lstT = {}

		err = 0

		for k,v in ipairs(lstF) do
			if v.tagged == 1 then
				if v.directory == 1 then
					path1 = GetCurrentWorkingDirectory()

					SetCurrentWorkingDirectory(v.name)

					SwitchPanes()
					path2 = GetCurrentWorkingDirectory()

					CreateDirectory(v.name)
					--QueueFileOp(eOp_MakeDirectory, v.name);

					SetCurrentWorkingDirectory(v.name)
					SwitchPanes()
					if TagAll() > 0 then
						err = err + __TagCopyX(command, buff)
					end

					SetCurrentWorkingDirectory(path1)
					SwitchPanes()
					SetCurrentWorkingDirectory(path2)
					SwitchPanes()
				else
					lstT[1+#lstT] = v
					QueueFileOp(eOp_Copy, v.name)
				end
			end
		end
		lstF = nil

		if #lstT > 0 then
			lstF, errc = DoFileOps()
			err = err + errc
			lstT = nil
			for k, v in ipairs(lstF) do
				if v.result_code == 0 then
					file_count = file_count + 1
					file_size = file_size + v.length
				end
				buff[1+#buff] = "Copy " .. v.result_msg ..  " on " .. v.source_path .. "/" .. v.source_filename
			end

			buff[1 + #buff] = ""
			buff[1 + #buff] = "Copied " .. file_count .. " files at " .. size_mash(file_size)
		end

		return err
	end


	function __TagMoveX(command, buff)
		local lstF
		local lstT
		local k,v, err, errc
		local file_size
		local file_count

		file_size = 0
		file_count = 0

		lstF = GetTaggedFileList()
		lstT = {}
		err = 0

		for k,v in ipairs(lstF) do
			if v.tagged == 1 then
				if v.directory == 1 then
					local path1
					local path2

					path1 = GetCurrentWorkingDirectory()
					SwitchPanes()
					path2 = GetCurrentWorkingDirectory()
					CreateDirectory(v.name)
					SetCurrentWorkingDirectory(v.name)
					SwitchPanes()
					SetCurrentWorkingDirectory(v.name)

					if TagAll() > 0 then
						err = err + __TagMoveX(command, buff)
					end

					ChangeDirUp()
					RemoveDirectory(v.name)
					SetCurrentWorkingDirectory(path1)
					SwitchPanes()
					SetCurrentWorkingDirectory(path2)
					SwitchPanes()
				else
					lstT[1+#lstT]=v
					QueueFileOp(eOp_Move, v.name)
				end
			end
		end
		lstF = nil

		if #lstT > 0 then
			lstF, errc = DoFileOps()
			err = err + errc
			lstT = nil
			for k, v in ipairs(lstF) do
				if v.result_code == 0 then
					file_count = file_count + 1
					file_size = file_size + v.length
				end
				buff[1+#buff] = "Move " .. v.result_msg ..  " on " .. v.source_path .. "/" .. v.source_filename
			end
			buff[1 + #buff] = ""
			buff[1 + #buff] = "Moved " .. file_count .. " files at " .. size_mash(file_size)
		end

		return err
	end

	function __TagDeleteX(command, buff)
		local lstF
		local lstT
		local err
		local k,v, err, errc

		local file_size
		local file_count

		file_size = 0
		file_count = 0

		lstF = GetTaggedFileList()
		lstT = {}

		err = 0

		for k,v in ipairs(lstF) do
			if v.tagged == 1 then
				if v.directory == 1 then
					local path1

					path1 = GetCurrentWorkingDirectory()
					SetCurrentWorkingDirectory(v.name)

					if TagAll() > 0 then
						err = err + __TagDeleteX(command, buff)
					end

					ChangeDirUp()
					RemoveDirectory(v.name)
					SetCurrentWorkingDirectory(path1)
				else
					lstT[1+#lstT]=v
					QueueFileOp(eOp_Delete, v.name)
				end
			end
		end
		lstF = nil

		if #lstT > 0 then
			lstF, errc = DoFileOps()
			err = err + errc
			lstT = nil
			for k, v in ipairs(lstF) do
				if v.result_code == 0 then
					file_count = file_count + 1
					file_size = file_size + v.length
				end
				buff[1+#buff] = "Delete " .. v.result_msg ..  " on " .. v.source_path .. "/" .. v.source_filename
			end
			buff[1 + #buff] = ""
			buff[1 + #buff] = "Delted " .. file_count .. " files at " .. size_mash(file_size)
		end

		return err
	end

	function __TagCopy(command)
		local buff = {}
		if #GetTaggedFileList() == 0 then TagHighlightedFile() end
		if __TagCopyX(command, buff) > 0 or GetOption("copy_move", "display_log") == "true" then
			ViewLuaTable("COPY OPERATIONS LOG", buff);
		end
	end
	function __TagMove(command)
		local buff = {}
		if #GetTaggedFileList() == 0 then TagHighlightedFile() end
		__TagMoveX(command, buff)
		if __TagCopyX(command, buff) > 0 or GetOption("copy_move", "display_log") == "true" then
			ViewLuaTable("MOVE OPERATIONS LOG", buff);
		end
	end
	function __TagDelete(command)
		local buff = {}
		if #GetTaggedFileList() == 0 then TagHighlightedFile() end
		__TagDeleteX(command, buff)
		if __TagCopyX(command, buff) > 0 or GetOption("delete", "display_log") == "true" then
			ViewLuaTable("DELETE OPERATIONS LOG", buff);
		end
	end

	function __TagFilter(command)
		TagWithFilter(trim(command))
	end

	function __TagGlob(command)
		TagWithGlob(trim(command))
	end

	function __TagUnTagAll(command)
		ClearAllTags();
	end

	function __TagAll(command)
		TagAll();
	end

	function __TagAllFiles(command)
		lstF = GetFileList()
		for k,v in ipairs(lstF) do
			if v.tagged == 0 and v.directory == 0 then
				TagFile(v.name)
			end
		end
	end

	function __TagAllDirs(command)
		lstF = GetFileList()
		for k,v in ipairs(lstF) do
			if v.tagged == 0 and v.directory == 1 then
				TagFile(v.name)
			end
		end
	end

	function __TagFlip(command)
		TagFlip();
	end


	function CLIParse(command)
		local cmd
		local finished
		local k, v

		finished = false
		cmd = command .. " "


		for k,v in pairs(cmds) do
			if string.sub(cmd, 1, #k) == k then
				v(string.sub(cmd, #k+1, #cmd-1))
				finished = true
				break
			end
		end

		if finished == false then
			debug_msg("Unknown command : " .. cmd)
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
		local dsortfunc

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


		-- name ascending
		dsortfunc = function(a,b) return a.data.name<b.data.name end
		if opt_sort_order == "name_desc" then
			sortfunc = function(a,b) return a.data.name>b.data.name end
			dsortfunc = sortfunc
		elseif opt_sort_order == "name_asc" then
			sortfunc = function(a,b) return a.data.name<b.data.name end
			dsortfunc = sortfunc
		elseif opt_sort_order == "size_desc" then
			sortfunc = function(a,b) return a.data.size>b.data.size end
		elseif opt_sort_order == "size_asc" then
			sortfunc = function(a,b) return a.data.size<b.data.size end
		elseif opt_sort_order == "date_asc" then
			sortfunc = function(a,b) return a.data.date<b.data.date end
			dsortfunc = sortfunc
		elseif opt_sort_order == "date_desc" then
			sortfunc = function(a,b) return a.data.date>b.data.date end
			dsortfunc = sortfunc
		else
			-- default use name ascending...
			sortfunc = function(a,b) return a.data.name<b.data.name end
		end

		if opt_dirs_first == "true" then
			-- sort by name for dirs always
			table.sort(lstSD, dsortfunc)

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

	function LoadPlugins()
		local k, v, lstPlugins

		lstPlugins = GetFileListFromPath("$HOME/.alfc");

		for k, v in pairs(lstPlugins) do
			--if DoesFileExist("$HOME/.alfc/core_extract.lua") == 0 then
			--IncludeFile("$HOME/.alfc/core_extract.lua")
			--end

			-- match 'core_' * '.lua'
			if string.find(v.name, "^(core_).*([.]lua)$") ~= nil then


				IncludeFile(string.gsub(v.path .. pathsep .. v.name, "\\", "/"))
			end
		end
		lstPlugins = nil
	end

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

		cmds[":td "] = __TagDelete
		cmds[":tc "] = __TagCopy
		cmds[":tm "] = __TagMove

		cmds[":tf "] = __TagFilter
		cmds[":tg "] = __TagGlob
		cmds[":ta "] = __TagAll
		cmds[":taf "] = __TagAllFiles
		cmds[":tad "] = __TagAllDirs
		cmds[":tu "] = __TagUnTagAll
		cmds[":t! "] = __TagFlip


	LoadPlugins()

	debug_msg("Global Lua Functions bootstrapped")
	BindKey(ALFC_KEY_F01, "View", [[ViewFile(GetHighlightedFilename())]])

	--BindKey(ALFC_KEY_F02, "Same", [[:s]])
	--BindKey(ALFC_KEY_F03, "History", [[ViewHistory()]])

	BindKey(ALFC_KEY_ALT + string.byte("X"), "Quit", [[:q]])
	--BindKey(ALFC_KEY_CTRL + string.byte("X"), "Quit", [[:q]])

	BindKey(ALFC_KEY_F12, "Tag", [[TagHighlightedFile()]])

	BindKey(ALFC_KEY_ALT + string.byte("C"), "Copy Tagged", [[:tc]])
	BindKey(ALFC_KEY_ALT + string.byte("D"), "Del Tagged", [[:td]])
	BindKey(ALFC_KEY_ALT + string.byte("M"), "Move Tagged", [[:tm]])

	BindKey(ALFC_KEY_ALT + string.byte("A"), "About", [[About()]])
	BindKey(ALFC_KEY_ALT + string.byte("H"), "Home", [[SetCurrentWorkingDirectory("$HOME")]])

	-- Dont bind to common keys you will need to use in the command line bar..
	--BindKey(ALFC_KEY_DEL, "Del", [[QueueFileOp(eOp_Delete, GetHighlightedFilename()); DoFileOps();]])
	--BindKey(ALFC_KEY_INS, "Copy", [[QueueFileOp(eOp_Copy, GetHighlightedFilename()); DoFileOps();]])

	-- Sometimes I want a quick view for code files
	BindKey(ALFC_KEY_F11, "CodeOnly", [[SetFilter("\\.[ch]$"); SetGlob("*.lua")]])

	CreateMenu(ALFC_KEY_ALT + string.byte("F"), "File", {
				{ key = 'X', name = "Exit", code = [[__QuitApp()]]},
				{ key = 'A', name = "About", code = [[About()]] },
				{ key = 'F1', name = "Help", code = [[ViewFile("help.txt")]] },
			})

	CreateMenu(ALFC_KEY_ALT + string.byte("1"), "Left Panel", {
				{ key = 'S', name = "make Same as Right", code = [[__MakeInactivePaneSame()]]},
				{ key = 'A', name = "About", code = [[About()]] },
			})

	CreateMenu(ALFC_KEY_ALT + string.byte("2"), "Right Panel", {
				{ key = 'S', name = "make Same as left", code = [[__MakeInactivePaneSame()]]},
			})

	_G["DIR_BOOTSTRAP"] = 1
end -- _G["BOOTSTRAP"]

