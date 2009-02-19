-- This function takes a list of files, applies a map to each one for an md5 has sum

cmds[":md5 "] = function()
	local buff = {}
	local k, v, t

	local map_foo = function (a)
		if a.directory == 1 then return nil end

		local f = io.open(a.name);
		local c = f:read("*all");
		local h = MD5Sum(c);

		f:close();
		return h .. "  " .. a.name;
	end

	t = GetTaggedFileList()
	if #t == 0 then
		t = GetHighlightedFile()
	end

	map(t, map_foo)

	for k,v in pairs(t) do
		buff[1+#buff] = v
	end

	ViewLuaTable("MD5Sum Hash Buffer", buff);
end

cmds[":sha "] = function()
	local buff = {}
	local k, v, t

	local map_foo = function (a)
		if a.directory == 1 then return nil end

		local f = io.open(a.name);
		local c = f:read("*all");
		local h = SHA1Sum(c);

		f:close();
		return h .. "  " .. a.name;
	end

	t = GetTaggedFileList()
	if #t == 0 then
		t = GetHighlightedFile()
	end

	map(t, map_foo)

	for k,v in pairs(t) do
		buff[1+#buff] = v
	end

	ViewLuaTable("SHA1Sum Hash Buffer", buff);
end


cmds[":ripemd "] = function()
	local buff = {}
	local k, v, t

	local map_foo = function (a)
		if a.directory == 1 then return nil end

		local f = io.open(a.name);
		local c = f:read("*all");
		local h = RIPEMD160Sum(c);

		f:close();
		return h .. "  " .. a.name;
	end

	t = GetTaggedFileList()
	if #t == 0 then
		t = GetHighlightedFile()
	end

	map(t, map_foo)

	for k,v in pairs(t) do
		buff[1+#buff] = v
	end

	ViewLuaTable("RIPEMD-160 Hash Buffer", buff);
end


