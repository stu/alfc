
cmds[":x "] = function (command)
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
	decomp[".zip"] = { exec = "unzip", parms = "-o" }
	decomp[".rar"] = { exec = "unrar", parms = "x -o+" }
	decomp[".zoo"] = { exec = "zoo", parms = "xO" }
	decomp[".lha"] = { exec = "lha", parms = "xf" }
	decomp[".arc"] = { exec = "arc", parms = "xo" }
	-- arj allows us to reset the file attributes on overwriting "-ha"
	decomp[".arj"] = { exec = "arj", parms = "x -jyo -ha" }

	flist = GetTaggedFileList()
	if #flist == 0 then
		flist = GetHighlightedFile()
	end

	for k,v in pairs(flist) do
		if v.directory == 0 then
			ext = findpattern(v.name, "[.]")
			if decomp[ext] ~= nil then
				debug_msg("" .. decomp[ext].exec .. " " .. decomp[ext].parms .. " " .. v.path .. pathsep .. v.name .. " ")
			else
				debug_msg("Don't know how to extract " .. v.name)
			end
		end

	end
end


