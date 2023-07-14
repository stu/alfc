#!/usr/bin/env ruby
#
# BuildVersion 0.2 - Ruby Version by Stu George
# http://mega-tokyo.com
#
# Author:: Stu George
# License:: Public Domain
# Copyright:: blah blah
#
# v0.2 - 20041106
# - Added options to increment major/minor/build numbers
#
# v0.1 - 20041105
#  - Initial version in Ruby 1.8.2 (2004-07-29) [i386-mswin32]

#
if __FILE__ == $0
require 'getoptlong'
end

# This does all the heavy lifting of the version file
#
class BuildVersion
	attr_reader :major, :minor, :build
	attr_writer :major, :minor, :build

	#constructor
	def initialize(filename, major, minor, build)
		@major = major
		@minor = minor
		@build = build
		@filename = filename

		# read version file into memory, if it exists.
		if File.exist?(@filename) == true
			lx = IO.readlines(@filename)

			# parse lines
			lx.each do |line|
				lx = line.split("=")
				if lx[0] == "VERSION"
					# ... do nothing.
				elsif lx[0] == "VER_MAJ"
					@major = lx[1].to_i
				elsif lx[0] == "VER_MIN"
					@minor = lx[1].to_i
				elsif lx[0] == "VER_BUILD"
					@build = lx[1].to_i
				else
					STDERR.puts "Error! Not a VERSION file I created."

					@major = major
					@minor = minor
					@build = build
				end
			end
		end

	end

	# serialise
	def to_s
		"BuildVersion: v#{@major}.#{@minor}.#{build}"
	end

	def IncrementBuild
		@build = @build + 1
	end

	def IncrementMinor
		@minor = @minor + 1
		@build = 0
	end

	def IncrementMajor
		@major = @major + 1
		@minor = 0
		@build = 0
	end

	def WriteVersion
		# Delete file if it exists
		if File.exist?(@filename) == true
			File.delete(@filename)
		end

		# Open a new one and write, only if it succeeded
		fp = File.new(@filename, "w")
		if fp != nil
			fp.puts "VERSION=#{@major}.#{@minor}.#{@build}"
			fp.puts "VER_MAJ=#{@major}"
			fp.puts "VER_MIN=#{@minor}"
			fp.puts "VER_BUILD=#{@build}"
			fp.close
		end

		if File.exist?("version.h") == false
			# Open a new one and write, only if it succeeded
			fp = File.new("version.h", "w")
			if fp != nil
				fp.puts "#ifndef VERSION_H\n"
				fp.puts "#define VERSION_H\n"
				fp.puts "\n"
				fp.puts "#ifdef __cplusplus\n"
				fp.puts "extern \"C\"{\n"
				fp.puts "#endif\n"
				fp.puts "\n"
				fp.puts "extern uint16_t VersionMajor(void);\n"
				fp.puts "extern uint16_t VersionMinor(void);\n"
				fp.puts "extern uint16_t VersionBuild(void);\n"
				fp.puts "extern const char* VersionString(void);\n"
				fp.puts "extern const char* VersionTime(void);\n"
				fp.puts "extern const char* VersionDate(void);\n"
				fp.puts "\n"
				fp.puts "#ifdef __cplusplus\n"
				fp.puts "};\n"
				fp.puts "#endif\n"
				fp.puts "#endif\n\n"
			end
		end

		if File.exist?("version.c") == true
			File.delete("version.c")
		end
		fp = File.new("version.c", "w")
		if fp != nil
			fp.puts "#include <stdint.h>\n"
			fp.puts "#include \"version.h\"\n\n"
			fp.puts "const char* VersionTime(void)\n"
			fp.puts "{\n"
			fp.puts "\treturn __TIME__;\n"
			fp.puts "}\n"
			fp.puts "const char* VersionDate(void)\n"
			fp.puts "{\n"
			fp.puts "\treturn __DATE__;\n"
			fp.puts "}\n"
			fp.puts "uint16_t VersionMajor(void)\n"
			fp.puts "{\n"
			fp.puts "	return #{@major};\n"
			fp.puts "}\n"
			fp.puts "\n"
			fp.puts "uint16_t VersionMinor(void)\n"
			fp.puts "{\n"
			fp.puts "\treturn #{@minor};\n"
			fp.puts "}\n"
			fp.puts "\n"
			fp.puts "uint16_t VersionBuild(void)\n"
			fp.puts "{\n"
			fp.puts "\treturn #{@build};\n"
			fp.puts "}\n"
			fp.puts "\n"
			fp.puts "const char* VersionString(void)\n"
			fp.puts "{\n"
			fp.puts "\treturn (char*)\"v#{@major}." + ("00" + @minor.to_s).slice(-2,2) + "/" + ("0000" + @build.to_s).slice(-4,4) + "\";\n"
			fp.puts "}\n\n"
			fp.close
		end
	end
end

if __FILE__ == $0
# This is the main starter class that just instantiates a BuildVersion
# and runs it.
#
class Application

	def initialize
		opts = GetoptLong.new(
				[ "--major",    "-M",			GetoptLong::NO_ARGUMENT ],
				[ "--minor", 	"-m",			GetoptLong::NO_ARGUMENT ],
				[ "--build",   	"-b",			GetoptLong::NO_ARGUMENT ],
				[ "--help",		"-h", "-?",		GetoptLong::NO_ARGUMENT ]
			)

		@func = 3
		opts.each do |opt, arg|
			# puts "Option: #{opt}, arg #{arg.inspect}"
			if opt == "--major"
				@func = 1
			elsif opt == "--minor"
				@func = 2
			elsif opt == "--build"
				@func = 3
			else
				puts "Syntax: BuildVersion -Mmbh?"
				puts "--major	-M	Increment major version number"
				puts "--minor	-m	Increment minor version number"
				puts "--build	-b	Increment build number, *default*"
				puts "--help	-h	This screen"

				exit 1
			end
		end

	end

	def start
		# create a new version, or if exists, read existing version file.
		cBV = BuildVersion.new("version", 0,1,0)

		# increment build number
		case @func
			when 1
				cBV.IncrementMajor

			when 2
				cBV.IncrementMinor

			when 3
				cBV.IncrementBuild
		end

		# debugging really. no need to display.
		puts cBV.to_s

		# save to disk.
		cBV.WriteVersion
	end
end

# Create new app and run!
AppMain = Application.new
AppMain.start
end
