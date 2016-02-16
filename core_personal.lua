
-- X11 can bind well to alt/ctrl whereas ncurses has trouble with some bindings
if DriverName() == "X11" then
	-- Sometimes I want a quick view for code files
	BindKey(ALFC_KEY_CTRL + string.byte('C'), "Code", [[SetFilter("\\.[ch]$"); SetGlob("*.lua")]])
	BindKey(ALFC_KEY_CTRL + string.byte('D'), "Docs", [[SetFilter(""); SetGlob("*.txt"); AddGlob("*.doc"); AddGlob("README");]])
end


function personal_TagAndDelBAK()
	TagWithGlob("*.bak")
	--TagWithGlob("*~")
	
	-- dont tag directories
	lstF = GetFileList()
	for k,v in ipairs(lstF) do
		if v.tagged == 1 and v.directory == 1 then
			-- this will flip the tag
			TagFile(v.name)
		end
	end
	
	__TagDelete("")
end

AddCommand(":delbak ","(Personal) Delete bak files", personal_TagAndDelBAK)
