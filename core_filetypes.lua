
-- This allows application of colour to a fileglob
function DecomposeFiletypes()
	local filetypes = {}

	filetypes.images = {}
	filetypes.images.extensions = { "*.gif", "*.png", "*.jpg" }
	filetypes.images.type = FILETYPE_IMAGE

	filetypes.archives = {}
	filetypes.archives.extensions = { "*.tar", "*.taz", "*.tar.gz", "*.tar.bz", "*.tar.bz2", "*.tgz", "*.tbz", "*.zip", "*.lha", "*.rar", "*.arc" }
	filetypes.archives.type = FILETYPE_ARCHIVE

	filetypes.docs = {}
	filetypes.docs.extensions = { "README", "INSTALL", "*.txt", "*.doc" }
	filetypes.docs.type = FILETYPE_DOC

	filetypes.backup = {}
	filetypes.backup.extensions = { "*.bak", "*.backup", "*~" }
	filetypes.backup.type = FILETYPE_BACKUP

	local k, v
	local kk, vv

	for k,v in pairs(filetypes) do
		for kk, vv in pairs(v.extensions) do
			ftypes[vv] = {}
			ftypes[vv].type = v.type
		end
	end
end

DecomposeFiletypes()

