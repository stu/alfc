-- This function takes a list of files, applies a map to each one for an md5 has sum

function foo(a)
	local s

	if a.directory == 1 then return nil end

	status("Hashing " .. a.name)
	local f = io.open(a.name);
	local c = f:read("*all");

	s = MD5Sum(c) .. "  " .. RIPEMD160Sum(c) .. "  " .. SHA1Sum(c) .. "  " .. a.name

	--local h = MD5Sum(c);
	f:close();
	--return h .. "  " .. a.name;
	return s
end

local k,v, t, buff

t = GetTaggedFileList()
map(t, foo)

buff = {}

buff[1+#buff] = "MD5                               RIPEMD-160                                SHA-1"
for k,v in pairs(t) do
	buff[1+#buff] = v
end

ViewLuaTable("MD5Sum Hash Buffer", buff);

