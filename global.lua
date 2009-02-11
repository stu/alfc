--[[

	Global Lua Functions, extra commands that can be called by ALFC

	Contains the global functions (separate) for
		Directory Mode,
]]

if _G["DIR_BOOTSTRAP"] ~= 1 and GetMode() == eMode_Directory then

	function x(fname)
		local f
		local c
		f = io.open(fname)
		c = f:read("*all")
		debug_msg(MD5Sum(c))
		f:close()
	end

	-- merge two tables together
	local function merge(t, u)
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
	local function __QuitApp()
		SetQuitAppFlag(1)
	end

	local function __FilterAdd(command)
		local cmd

		cmd = trim(command)
		AddFilter(cmd)
	end

	local function __Filter(command)
		local cmd

		cmd = trim(command)
		SetFilter(cmd)
	end

	local function __GlobAdd(command)
		local cmd

		cmd = trim(command)
		AddGlob(cmd)
	end

	local function __Glob(command)
		local cmd

		cmd = trim(command)
		SetGlob(cmd)
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
	local function __Jump(command)
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

	local function __ChangeDir(command)
		local cmd = command

		-- try to deal with spaces
		if( SetCurrentWorkingDirectory(command) == -1) then
			SetCurrentWorkingDirectory(trim(command))
		end
	end

	local function __SortOrder(command)
		if trim(command) == "na" then
			SetOption("options", "sort_order", "name_asc")
		elseif trim(command) == "nd" then
			SetOption("options", "sort_order", "name_desc")
		elseif trim(command) == "sa" then
			SetOption("options", "sort_order", "size_asc")
		elseif trim(command) == "sd" then
			SetOption("options", "sort_order", "size_desc")
		end

		-- re-sort..
		SortFileList()
		RedrawWindow()
	end

	local function __TagCopy(command)
		local lstF
		local lstT
		local k,v, err

		local path1, path2

		lstF = GetFileList()
		lstT = {}

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
					TagAll()
					__TagCopy(command)

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

		--debug_msg("Copying " .. #lstT .. " tagged files")
		if #lstT > 0 then
			lstF, err = DoFileOps()
			--[[
			if err > 0 then
				lstT = {}
				for k, v in ipairs(lstF) do
					lstT[1+#lstT] = "Copy on " .. v.source_path .. "/" .. v.source_filename .. " to " .. v.dest_path .. "/" .. v.dest_filename .. " = (" .. v.result_code .. ") " .. v.result_msg
				end

				ViewLuaTable(gd, "COPY OPERATIONS LOG", lstT);
			end
			]]
		end

	end

	local function __TagMove(command)
		local lstF
		local lstT
		local k,v

		lstF = GetFileList()
		lstT = {}

		for k,v in ipairs(lstF) do
			if v.tagged == 1 then
				lstT[1+#lstT]=v
				QueueFileOp(eOp_Move, v.name)
			end
		end
		lstF = nil

		debug_msg("Move " .. #lstT .. " tagged files")
		lstF, err = DoFileOps()
		for k, v in ipairs(lstF) do
			debug_msg("Move on " .. v.source_path .. "/" .. v.source_filename .. " to " .. v.dest_path .. "/" .. v.dest_filename .. " = (" .. v.result_code .. ") " .. v.result_msg)
		end
	end

	local function __TagDelete(command)
		local lstF
		local lstT
		local err
		local k, v, err

		lstF = GetFileList()
		lstT = {}

		for k,v in ipairs(lstF) do
			if v.tagged == 1 then
				if v.directory == 1 then
					local path1

					path1 = GetCurrentWorkingDirectory()
					SetCurrentWorkingDirectory(v.name)

					if TagAll() > 0 then
						__TagDelete(command)
					end

					SetCurrentWorkingDirectory(path1)
					RemoveDirectory(v.name)
					SetCurrentWorkingDirectory(path1)
				else
					lstT[1+#lstT]=v
					QueueFileOp(eOp_Delete, v.name)
				end
			end
		end
		lstF = nil

		debug_msg("Deleting " .. #lstT .. " tagged files")
		if #lstT > 0 then
			lstF, err = DoFileOps()
			--for k, v in ipairs(lstF) do
			--	debug_msg("delete on " .. v.source_path .. "/" .. v.source_filename .. " = (" .. v.result_code .. ") " .. v.result_msg)
			--end
		end
	end

	local function __TagFilter(command)
		TagWithFilter(trim(command))
	end

	local function __TagGlob(command)
		TagWithGlob(trim(command))
	end

	local function __TagUnTagAll(command)
		ClearAllTags();
	end

	local function __TagAll(command)
		TagAll();
	end

	local function __TagAllFiles(command)
		lstF = GetFileList()
		for k,v in ipairs(lstF) do
			if v.tagged == 0 and v.directory == 0 then
				TagFile(v.name)
			end
		end
	end

	local function __TagAllDirs(command)
		lstF = GetFileList()
		for k,v in ipairs(lstF) do
			if v.tagged == 0 and v.directory == 1 then
				TagFile(v.name)
			end
		end
	end

	local function __TagFlip(command)
		TagFlip();
	end


	function CLIParse(command)
		local cmd
		local cmds
		local finished
		local k, v

		finished = false
		cmd = command .. " "

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


		for k,v in pairs(cmds) do
			if string.sub(cmd, 1, #k) == k then
				v(string.sub(cmd, #k, #cmd))
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


		if opt_sort_order == "name_desc" then
			sortfunc = function(a,b) return a.data.name>b.data.name end
		elseif opt_sort_order == "name_asc" then
			sortfunc = function(a,b) return a.data.name<b.data.name end
		elseif opt_sort_order == "size_desc" then
			sortfunc = function(a,b) return a.data.size>b.data.size end
		elseif opt_sort_order == "size_asc" then
			sortfunc = function(a,b) return a.data.size<b.data.size end
		else
			-- default use name ascending...
			sortfunc = function(a,b) return a.data.name<b.data.name end
		end

		if opt_dirs_first == "true" then
			-- sort by name for dirs always
			table.sort(lstSD, function(a,b) return a.data.name<b.data.name end)

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

	--debug_msg("Global Lua Functions bootstrapped")
	BindKey(ALFC_KEY_F01, "View", [[ViewFile(GetHighlightedFilename())]])
	BindKey(ALFC_KEY_F02, "Same", [[:s]])
	BindKey(ALFC_KEY_F03, "History", [[ViewHistory()]])
	BindKey(ALFC_KEY_F10, "Quit", [[:q]])

	BindKey(ALFC_KEY_ALT + string.byte("C"), "Copy Tagged", [[:tc]])
	BindKey(ALFC_KEY_ALT + string.byte("D"), "Del Tagged", [[:td]])
	BindKey(ALFC_KEY_ALT + string.byte("M"), "Move Tagged", [[:tm]])

	-- Dont bind to common keys you will need to use in the command line bar..
	--BindKey(ALFC_KEY_DEL, "Del", [[QueueFileOp(eOp_Delete, GetHighlightedFilename()); DoFileOps();]])
	--BindKey(ALFC_KEY_INS, "Copy", [[QueueFileOp(eOp_Copy, GetHighlightedFilename()); DoFileOps();]])


	-- Sometimes I want a quick view for code files
	BindKey(ALFC_KEY_F11, "CodeOnly", [[SetFilter("\\.[ch]$"); SetGlob("*.lua")]])
	BindKey(ALFC_KEY_F12, "Tag", [[TagHighlightedFile()]])

	_G["DIR_BOOTSTRAP"] = 1
end -- _G["BOOTSTRAP"]

