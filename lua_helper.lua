-- This func scans game c files (speced in a table) and auto builds
-- func registrations. it looks for "int gme_" to start a function

function RegFuncs(nT, eF)
	local t
	local idx
	local line
	local xxx

	for xxx, tbl in pairs(eF) do

		for idx, line in pairs(tbl) do
			local q;

			q = "\tlua_register(l, \"" .. line .. "\"," .. string.rep(" ", 30 - #line) .. "gme_" .. line .. ");";
			table.insert(nT, q);
		end

	end

end

function ExternFuncs(nT, eF, idx)
	local lc = 0

	for idx, line in pairs(eF) do
		local q;

		q = "extern int gme_" .. line .. "(lua_State *L);";
		table.insert(nT, q);

		local ss = line .. string.rep(" ", 40)
		ss = string.sub(ss, 1, 30)

		if lc == 0 then
			io.stdout:write("   " .. ss);
		else
			io.stdout:write("" .. ss );
		end

		lc = lc + 1
		if lc == 4 then
			io.stdout:write("\n")
			lc = 0
		end
	end

	print("")

end

function ReadFileIntoTable(sF)
	local y = {};
	local line;

	for line in io.lines(sF) do
		table.insert(y, line);
	end

	return y;
end

function DeleteExtStuff(sT, idx)
	local k;
	local v;
	local skip = 0;
	local nT = {};

	for k,v in pairs(sT) do

		if v == "/* LUA END EXT REF */" then
			if idx > 0 then
				ExternFuncs(nT, funcs[idx]);
			else
				RegFuncs(nT, funcs);
			end
			skip = 0;
		end

		if skip == 0 then
			--table.remove(sT, k);
			table.insert(nT, v);
		end

		if v == "/* LUA EXT REF */" then
			skip = 1;
		end

	end

	return nT;
end

function ReadFuncs(sFN)
	local line;
	local idx;
	local l = {};
	local f = {};

	for line in io.lines(sFN) do
		table.insert(l, line)
	end

	for idx, line in pairs(l) do
		local smatch = "int gme_";
		local w;

		if string.sub(line, 1, #smatch)  == smatch then
			--print(line);
			local stab = {};
			local funcname = "";

			for w in string.gmatch(line, "[%a%d_]+") do
				table.insert(stab, w);
			end

			local funcname = string.sub(stab[2], 5);
			table.insert(f, funcname);

		end
	end

	return f;
end


funcs = {};
files_to_do = { "lua_api"
				}

for k,v in pairs(files_to_do) do
	local fname

	idx = #funcs + 1

	fname = "./" .. v;
	funcs[idx] = ReadFuncs(fname .. ".c");
	print("" .. fname .. "")
	lgf = DeleteExtStuff(ReadFileIntoTable(fname .. ".h_inc"), idx);

	local fd
	fd = io.output(fname .. ".h");
	for idx, line in pairs(lgf) do
		fd:write(line, "\n");
	end
	fd:close();
end


lgf = DeleteExtStuff(ReadFileIntoTable("./lua_helper.inc"), -1);

local fd
fd = io.output("./lua_helper.c");
for idx, line in pairs(lgf) do
	fd:write(line, "\n");
end
fd:close();

