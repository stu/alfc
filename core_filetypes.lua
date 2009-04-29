
-- This allows application of colour to a fileglob
function DecomposeFiletypes()
	local filetypes = {}

    -- by default this is used for anything the OS says is executable
    filetypes.exec = {}
    filetypes.exec.extensions = {"*.exe", "*.com", "*.bat" }
    filetypes.exec.type = FILETYPE_EXEC
    filetypes.exec.colr = "bright green"

    filetypes.movies = {}
    filetypes.movies.extensions = { "*.avi", "*.wmv", "*.mpg" }
    filetypes.movies.type = FILETYPE_MOVIE
    filetypes.movies.colr = "yellow"

	filetypes.images = {}
	filetypes.images.extensions = { "*.gif", "*.png", "*.jpg", "*.pcx", "*.lbm", "*.psd", "*.raw", "*.nef", "*.jpeg", "*.jpe" }
	filetypes.images.type = FILETYPE_IMAGE
    filetypes.images.color = "yellow"

	filetypes.archives = {}
	filetypes.archives.extensions = { "*.tar", "*.taz", "*.tar.gz", "*.tar.bz", "*.tar.bz2", "*.tgz", "*.tbz",
										"*.zip", "*.lha", "*.rar", "*.arc", "*.pak", "*.sit", "*.hqx" }
	filetypes.archives.type = FILETYPE_ARCHIVE
    filetypes.archives.color = "red"

	filetypes.docs = {}
	filetypes.docs.extensions = { "README", "INSTALL", "*.txt", "*.doc", "*.me", "*.log" }
	filetypes.docs.type = FILETYPE_DOC
    filetypes.docs.color = "green"

	filetypes.backup = {}
	filetypes.backup.extensions = { "*.bak", "*.backup", "*~", "*.old" }
	filetypes.backup.type = FILETYPE_BACKUP
    filetypes.backup.color = "dark grey"

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

