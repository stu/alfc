-- This function takes a list of files, applies a map to each one for an md5 has sum

function foo(a)
	if a.directory == 1 then return nil end

	local f = io.open(a.name);
	local c = f:read("*all");
	local h = MD5Sum(c);

	f:close();
	return h .. "  " .. a.name;
end

local k,v, t, buff

t = GetTaggedFileList()
map(t, foo)

buff = {}

for k,v in pairs(t) do
	buff[1+#buff] = v
end

ViewLuaTable("MD5Sum Hash Buffer", buff);

