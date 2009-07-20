
AddCommand(":ex ", "Extract archives", function(command)
--cmds[":ex "].func = function (command)
	local ext
	local flist
	local k,v
	local decomp = {}

	local findpattern = function(text, pattern, start)
							local p
							p = string.find(text, pattern, start)
							if p ~= nil then
								return string.sub(text, p, #text)
							else
								return ""
							end
						end

	-- tar + gzip
	decomp[".tar"] = { exec = "tar", parms = "x" }
	decomp[".taz"] = { exec = "tar", parms = "xfz" }
	decomp[".tgz"] = { exec = "tar", parms = "xfz" }
	decomp[".tar.gz"] = { exec = "tar", parms = "xfz" }

	-- tar + bzip2
	decomp[".tbz"] = { exec = "tar", parms = "xfj" }
	decomp[".tbz2"] = { exec = "tar", parms = "xfj" }
	decomp[".tar.bz"] = { exec = "tar", parms = "xfj" }
	decomp[".tar.bz2"] = { exec = "tar", parms = "xfj" }

	-- zip, arj, rar, zoo, lha
	-- switches force overwrite on extraction
	decomp[".zip"] = { exec = "unzip", parms = "-o" }  -- -d outputdir
	decomp[".rar"] = { exec = "unrar", parms = "x -o+" }
	decomp[".zoo"] = { exec = "zoo", parms = "x.O" } --
	decomp[".lha"] = { exec = "lha", parms = "xf" }
	decomp[".arc"] = { exec = "arc", parms = "xo" }
	-- arj allows us to reset the file attributes on overwriting "-ha"
	decomp[".arj"] = { exec = "arj", parms = "x -jyo -ha" }

	flist = GetTaggedFileList()
	if #flist == 0 then
		flist = GetHighlightedFile()
	end

	for k,v in pairs(flist) do
		local q = '\"'
		if v.directory == 0 then
			ext = findpattern(v.name, "[.]")
			if decomp[ext] ~= nil then
				local dcount = 0
				local dname = string.sub(v.name, 1, #v.name - #ext)
				if CreateDirectory(dname) == -1 then
					for dcount = 1,9999 do
						if CreateDirectory( dname .. string.sub("0000" .. dcount, -4)) == 0 then
							dname = dname .. string.sub("0000" .. dcount, -4)
							dcount = 9999
							break
						end
					end
				end

				local xdir = GetCurrentWorkingDirectory()

				SetCurrentWorkingDirectory(dname)
				dname = ("" .. decomp[ext].exec .. " " .. decomp[ext].parms .. " " .. q .. v.path .. pathsep .. v.name .. q .." ")
				--debug_msg(dname)
				execute(dname)
				SetCurrentWorkingDirectory(".")
			else
				debug_msg("Don't know how to extract " .. v.name)
			end
		end
	end
end
)

