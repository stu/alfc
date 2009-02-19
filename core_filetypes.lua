
-- This allows application of colour to a fileglob
function DecomposeFiletypes()
	local filetypes = {}

	filetypes.images = {}
	filetypes.images.extensions = { "*.gif", "*.png", "*.jpg" }
	filetypes.images.colour = "blue"

	filetypes.archives = {}
	filetypes.archives.extensions = { "*.tar", "*.taz", "*.tar.gz", "*.tar.bz", "*.tar.bz2", "*.tgz", "*.tbz", "*.zip", "*.lha", "*.rar", "*.arc" }
	filetypes.archives.colour = "light red"

	filetypes.docs = {}
	filetypes.docs.extensions = { "README", "INSTALL", "*.txt", "*.doc" }
	filetypes.docs.colour = "green"

	filetypes.backup = {}
	filetypes.backup.extensions = { "*.bak", "*.backup", "*~" }
	filetypes.backup.colour = "dark grey"


	local k, v
	local kk, vv

	for k,v in pairs(filetypes) do
		for kk, vv in pairs(v.extensions) do
			ftypes[vv] = v.colour
		end
	end
end

DecomposeFiletypes()

