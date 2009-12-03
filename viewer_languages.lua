
lang = {}

-- Python
lang.python = {}
lang.python.name = "Python"
lang.python.type = "Scripting"
lang.python.tab = 4
lang.python.tabs_to_spaces = true
lang.python.extensions = { ".py", ".pyc" }
lang.python.syntax = {}
lang.python.syntax.comment_line = { "#" }
lang.python.syntax.comment_pair = { { open = '"""', close = '"""'} }

-- Lua
lang.lua = {}
lang.lua.name = "Lua"
lang.lua.type = "Scripting"
lang.lua.tab = 4
lang.lua.tabs_to_spaces = false
lang.lua.extensions = { ".lua" }
lang.lua.syntax = {}
lang.lua.syntax.comment_line = { "--" }

-- Ruby
lang.ruby = {}
lang.ruby.name = "Ruby"
lang.ruby.type = "Scripting"
lang.ruby.tab = 4
lang.ruby.tabs_to_spaces = false
lang.ruby.extensions = { ".rb", ".rbc" }
lang.ruby.syntax = {}
lang.ruby.syntax.comment_line = { "#" }
lang.ruby.syntax.ops = { ">>=", "<<=","+=", "-=","*=","/=","^=","%=",">>","<<","&&","||","++","--","+","-","*","&","^","|","{","}" }

-- C / C++
lang.c = {}
lang.c.name = "C/C++"
lang.c.type = "Programming"
lang.c.tab = 4
lang.c.tabs_to_spaces = false
lang.c.extensions = { ".c", ".h", ".cc", ".cpp", ".hh", ".hpp" }
lang.c.syntax = {}
lang.c.syntax.keywords={"do","while","if","else","case","switch", "default", "break","return"}
lang.c.syntax.comment_line = { "//" }
lang.c.syntax.comment_pair = { { open="/*", close="*/"} }
lang.c.syntax.types = { "void", "int", "char", "short", "long", "float", "double", "unsigned", "static", "register" }
lang.c.syntax.ops = { ">>=", "<<=","+=", "-=","*=","/=","^=","%=",">>","<<","&&","||","++","--","+","-","*","&","^","|","{","}" }

-- Text
lang.text = {}
lang.text.name = "Text"
lang.text.type = "Plain Text"
lang.text.tab = 8
lang.text.tabs_to_spaces = false
lang.text.extensions = { ".txt", ".doc", ".me", ".log" }

-- Latex/Tex
lang.latex = {}
lang.latex.name = "Latex/Tex"
lang.latex.type = "Markup"
lang.latex.tab = 8
lang.latex.tabs_to_spaces = false
lang.latex.extensions = { ".tex" }
lang.latex.syntax = {"\\usepackage", "\\makeindex", "\\section", "\\subsection"}
lang.latex.syntax.comment_line = { "%" }
