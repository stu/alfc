--[[

	Global Lua Functions, extra commands that can be called by ALFC

	Contains the global functions (separate) for
		Directory Mode,
]]

if _G["DIR_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Directory then

	-- GLOBAL DEF
	cmds = {}		-- for quick commands
	ftypes = {}		-- allows colouring of filetypes via plugin

	function AddCommand(k, d, f)
		cmds[k] = {}
		cmds[k].desc = d
		cmds[k].func = f
	end

	function execute(text)
		local strCLI = "" .. text

		local _a_fn = GetHighlightedFilename()
		local _a_d = GetCurrentWorkingDirectory()
		local _a_t = GetTaggedFileList()
		if #_a_t > 0 then
			local _a_s = _a_t
		else
			local _a_s = _a_fn
		end

		SwitchPanes()
			local _n_fn = GetHighlightedFilename()
			local _n_d = GetCurrentWorkingDirectory()
			local _n_t = GetTaggedFileList()
			local _n_s
			if #_n_t > 0 then
				local _n_s = _n_t
			else
				local _n_s = _n_fn
			end
		SwitchPanes()


-- take from the ofm2004 standard page
-- * "%f" The current file name.
-- * "%F" The current file in the unselected panel.
-- * "%d" The current directory name.
-- * "%D" The directory name of the unselected panel.
-- * "%t" The currently tagged files.
-- * "%T" The tagged files in the unselected panel.
-- * "%u" and "%U" Similar to the %t and %T macros, but after the operation
--        all selected files are untagged. That means that this macro can be
--        used only once per menu entry (or extension entry), because on the
--        second and subsequent invocations there will be no tagged files.
-- * "%s" and "%S"  The tagged files if there are any. Otherwise the current file.
-- * "%cd" This is a special macro that is used to change the current directory to
--         the directory specified in front of it. This is used primarily as an
--         interface to the VFS.
-- * "%view" the invocation of the internal viewer. An argument to force the
--           viewer in a particular mode can be passed: ASCII to force the
--           viewer into ASCII mode; hex to force the viewer into hex mode;
-- * "%%" The % character
-- * "%{some text}" Popup an input box and prompts for the substitution.
--                  The user should be able to cancel input ( ESC or F10 recommended).

        _a_fn = string.gsub(_a_fn, " ", "\\ ")
        _n_fn = string.gsub(_n_fn, " ", "\\ ")
        _a_d = string.gsub(_a_d, " ", "\\ ")
        _n_d = string.gsub(_n_d, " ", "\\ ")

		strCLI = string.gsub(strCLI, "@f", _a_fn)
		strCLI = string.gsub(strCLI, "@F", _n_fn)
		strCLI = string.gsub(strCLI, "@d", _a_d)
		strCLI = string.gsub(strCLI, "@D", _n_d)

		-- how to pass in %t?? etc
		--debug_msg("exec [" .. strCLI .."]");
		if FunctionExists(strCLI) == 0 then
			ExecuteString(strCLI)
		else
			exec(strCLI)
		end
	end


	function findpattern(text, pattern, start)
		local p
		p = string.find(text, pattern, start)
		if p ~= nil then
			return string.sub(text, p, #text)
		else
			return ""
		end
	end

	function CreateMenu(key, name, list)
		local k,v, idx
		idx = AddMenu(key, name)

		for k,v in pairs(list) do
			AddMenuItem(idx, v.key, v.name, v.code)
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

	function __CreateDir(command)
        local cmd = command
        if #trim(command) == 0 then
            return
        end

        if( SetCurrentWorkingDirectory(command) == -1) then
            CreateDirectory(trim(command))
            SetCurrentWorkingDirectory(".")
        end
    end

	function __ChangeDir(command)
		local cmd = command

		-- if blank, trigger refresh
		if #trim(command) == 0 then
			SetCurrentWorkingDirectory(GetCurrentWorkingDirectory())
			return
		end

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


	function __TagSymlinkX(command, buff)
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
				--if v.directory == 1 then
				--else
					lstT[1+#lstT] = v
					QueueFileOp(eOp_SymLink, v.name)
				--end
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
				buff[1+#buff] = "Symlink " .. v.result_msg ..  " on " .. v.source_path .. "/" .. v.source_filename
			end

			buff[1 + #buff] = ""
		end

		return err
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
					local hflag

					hflag = GetHiddenFlag()
					SetHiddenFlag(0)

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

					SetHiddenFlag(hflag)

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
					local hflag

					hflag = GetHiddenFlag()
					SetHiddenFlag(0)

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

					SetHiddenFlag(hflag)
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
				if v.directory == 1 and v.link == 0 then
					local path1
					local hflag

					hflag = GetHiddenFlag()
					SetHiddenFlag(0)

					path1 = GetCurrentWorkingDirectory()
					SetCurrentWorkingDirectory(v.name)

					if TagAll() > 0 then
						err = err + __TagDeleteX(command, buff)
					end

					SetHiddenFlag(hflag)

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

	function __TagSymlink(command)
		local buff = {}
		if #GetTaggedFileList() == 0 then TagHighlightedFile() end
		if __TagSymlinkX(command, buff) > 0 then
			ViewLuaTable("SYMLINK OPERATIONS LOG", buff);
		end
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
		if __TagMoveX(command, buff) > 0 or GetOption("copy_move", "display_log") == "true" then
			ViewLuaTable("MOVE OPERATIONS LOG", buff);
		end
	end
	function __TagDelete(command)
		local buff = {}
		if #GetTaggedFileList() == 0 then TagHighlightedFile() end
		if __TagDeleteX(command, buff) > 0 or GetOption("delete", "display_log") == "true" then
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
				v.func(string.sub(cmd, #k+1, #cmd-1))
				finished = true
				break
			end
		end

		if finished == false then
			if string.sub(command, 1, 1) == ":" then
				execute(string.sub(command, 2, #command))
			else
				execute(command)
			end
			--debug_msg("Unknown command : " .. cmd)
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
			sortfunc = function(a,b) return string.lower(a.data.name) > string.lower(b.data.name) end
			dsortfunc = sortfunc
		elseif opt_sort_order == "name_asc" then
			sortfunc = function(a,b) return string.lower(a.data.name) < string.lower(b.data.name) end
			dsortfunc = sortfunc
		elseif opt_sort_order == "size_desc" then
			sortfunc = function(a,b) return a.data.size > b.data.size end
		elseif opt_sort_order == "size_asc" then
			sortfunc = function(a,b) return a.data.size < b.data.size end
		elseif opt_sort_order == "date_asc" then
			sortfunc = function(a,b) return a.data.date < b.data.date end
			dsortfunc = sortfunc
		elseif opt_sort_order == "date_desc" then
			sortfunc = function(a,b) return a.data.date > b.data.date end
			dsortfunc = sortfunc
		else
			-- default use name ascending...
			sortfunc = function(a,b) return string.lower(a.data.name) < string.lower(b.data.name) end
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
			local kk,vv
			AddToSortList(v.data.name)
			SetFileType(v.data.name, 0)

			for kk,vv in pairs(ftypes) do
				if globmatch(string.lower(v.data.name), string.lower(kk)) == 0 then
					SetFileType(v.data.name, vv.type)
					break
				end
			end
		end
	end

	function LoadPlugins()
		local k, v, lstPlugins

		lstPlugins = GetFileListFromPath("$HOME/.alfc");
		table.sort(lstPlugins, function(a,b) return a.name<b.name end)

		for k, v in pairs(lstPlugins) do
			-- match 'core_' * '.lua'
			if string.find(v.name, "^(core_).*([.]lua)$") ~= nil then
				IncludeFile(string.gsub(v.path .. pathsep .. v.name, "\\", "/"))
			end
		end
		lstPlugins = nil
	end

	function __SwapPanels(command)
		local p1, p2

		p1 = GetCurrentWorkingDirectory()
		SwitchPanes()
		p2 = GetCurrentWorkingDirectory()
		SetCurrentWorkingDirectory(p1)
		SwitchPanes()
		SetCurrentWorkingDirectory(p2)
	end

	cmds = {}
	AddCommand(":q ", "Quit ALFC", __QuitApp)
	AddCommand(":f ", "Filter file list with regexp", __Filter)
	AddCommand(":f+ ","Add regexp to filter list",	__FilterAdd)
	AddCommand(":g ","Filter file list with glob", __Glob)
	AddCommand(":g+ ","Add glob to filter list",__GlobAdd)
	AddCommand(":s ","Make both panels same", __MakeInactivePaneSame)
	AddCommand(":j ","Jump", __Jump)
	AddCommand(":cd ","Change directory", __ChangeDir)
	AddCommand(":md ","Create directory", __CreateDir)
	AddCommand(":so ","Set sort order", __SortOrder)
	AddCommand(":td ","Delete tagged files", __TagDelete)
	AddCommand(":tc ","Copy tagged files", __TagCopy)
	AddCommand(":tm ","Move tagged files", __TagMove)
	AddCommand(":tf ","Tag files via regexp", __TagFilter)
	AddCommand(":tg ","Tag files via glob", __TagGlob)
	AddCommand(":ta ","Tag all files AND directories", __TagAll)
	AddCommand(":taf ","Tag all files (but not dirs)", __TagAllFiles)
	AddCommand(":tad ","Tag all directories (but not files)", __TagAllDirs)
	AddCommand(":tu ","Untag all", __TagUnTagAll)
	AddCommand(":t! ","Flip tags",	__TagFlip)
	AddCommand(":swap ","Swap active panels", __SwapPanels)

	if SystemType() ~= "WIN32" then
		AddCommand(":sym","Symlink files", __TagSymlink)
	end

	LoadPlugins()

	--------------------------------------------------------------------------
	IncludeFile("$HOME/.alfc/filemanager_menu.lua")

	_G["DIR_BOOTSTRAP"] = 1
end -- _G["BOOTSTRAP"]



