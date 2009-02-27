
-- This allows application of colour to a fileglob
function DecomposeFiletypes()
	local filetypes = {}

	filetypes.images = {}
	filetypes.images.extensions = { "*.gif", "*.png", "*.jpg", "*.pcx", "*.lbm", "*.psd", "*.raw", "*.nef", "*.jpeg", "*.jpe" }
	filetypes.images.type = FILETYPE_IMAGE

	filetypes.archives = {}
	filetypes.archives.extensions = { "*.tar", "*.taz", "*.tar.gz", "*.tar.bz", "*.tar.bz2", "*.tgz", "*.tbz",
										"*.zip", "*.lha", "*.rar", "*.arc", "*.pak", "*.sit", "*.hqx" }
	filetypes.archives.type = FILETYPE_ARCHIVE

	filetypes.docs = {}
	filetypes.docs.extensions = { "README", "INSTALL", "*.txt", "*.doc", "*.me" }
	filetypes.docs.type = FILETYPE_DOC

	filetypes.backup = {}
	filetypes.backup.extensions = { "*.bak", "*.backup", "*~", "*.old" }
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

