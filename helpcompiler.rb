#!/usr/bin/ruby

#
# Hacky ruby script to compile the help text into our help file format.
#

require 'zlib'

class HelpSection
	attr_accessor :name, :lines
	def initialize
		@name = ""
		@lines = Array.new()
	end
end

class HelpFile
	attr_accessor :author, :revision, :title, :sections

	def initialize
		@author = ""
		@revision = ""
		@title = ""
		@sections = Array.new()

		@in_size = 0
		@out_size = 0
	end


	def bdump(text, record, fp)
		fp.putc record.to_i
		fp.putc text.length.to_i >> 8
		fp.putc text.length.to_i & 0xFF

		@in_size += text.length
		@out_size += text.length

		text.each_byte do |x|
			fp.putc x
		end
	end


	def deflate(string, level)
		z = Zlib::Deflate.new(level)
		dst = z.deflate(string, Zlib::FINISH)
		z.close
		return dst
	end

	def inflate(string)
		zstream = Zlib::Inflate.new
		buf = zstream.inflate(string)
		zstream.finish
		zstream.close
		return buf
	end


	def bdump_gz(text, record, fp)
		xx = deflate(text, 9)

		fp.putc record.to_i + 0x20.to_i

		fp.putc xx.length.to_i >> 8
		fp.putc xx.length.to_i & 0xFF

		fp.putc text.length.to_i >> 8
		fp.putc text.length.to_i & 0xFF

		@in_size += text.length
		@out_size += xx.length

		xx.each_byte do |x|
			fp.putc x
		end


		a1 = text.length.to_f
		a2 = xx.length.to_f
		if a1 == 0 then a1 = 1; end

		#puts "in " + text.length.to_s + " out = " + xx.length.to_s + " as " + ((a2 / a1) * 100).to_i.to_s + "%"
	end

	def outline(x, ofp)
		xx = x.split("\n")

		xx.each do |xxx|
			q = xxx.rstrip
			bdump_gz(q, 0x05, ofp)
		end
		return ""
	end

	def bust_attr(s, a)
		b = s.to_s.index(a)
		if b == nil then return "" end

		if b >= 0 then
			x = s[(b + a.length) .. (b+a.length) + s[(b+a.length)..-1].index("}") - 1].to_s
		end
	end

	def build_list(lname)
		qq = Array.new()
		if lname == "*" then
			@sections.each do |s|
				ss = s.name.split(":")
				st = "  " * ss.length
				qq << "#{st}\\link{#{s.name}|#{ss[-1]}}"
			end
			return qq
		else
			n1 = lname + ":"
			n2 = lname + "}"

			@sections.each do |s|
				ss = s.name.split(":")
				st = "  " * ss.length

				if s.name.to_s[0 .. n1.length-1] == n1 then
					qq << "#{st}\\link{#{s.name}|#{ss[-1]}}"
				elsif s.name.to_s[0 .. n2.length-1] == n2 then
					qq << "#{st}\\link{#{s.name}|#{ss[-1]}}"
				elsif s.name.to_s[0 .. lname.length-1] == lname then
					qq << "#{st}\\link{#{s.name}|#{ss[-1]}}"
				end
			end
			return qq
		end
	end

	def compile(ofp)
		ofp.putc 0x28
		ofp.putc 0x4C

		bdump(@title, 0x01, ofp)
		bdump(@author, 0x02, ofp)
		bdump(@revision, 0x03, ofp)

		@sections.each do |s|
			puts "Section " + s.name
			bdump(s.name, 0x04, ofp)

			s.lines.each do |l|
				ll = bust_attr(l, "\\list{")
				while ll.length > 0 do
					if ll.length > 0 then
						qq = build_list(ll)
						q = l.index("\\list{")

						a1 = ""
						if q-1 > 0 then
							a1 = l[0 .. q-1]
						end
						a2 = l[q + 6 + ll.length + 1 .. -1]

						l = a1 + qq.join("\n") + a2
					end

					ll = bust_attr(l, "\\list{")
				end

				l.split("\n").each do |lx|
					outline(lx, ofp)
				end
			end
		end

		return @in_size, @out_size
	end
end



def PutHeaders(hlp, txt)
	txt.each do |t|
		if t.to_s.index("\\title{") != nil then
			t = hlp.bust_attr(t, "\\title{")
			hlp.title = t.to_s
		elsif t.to_s.index("\\author{") != nil then
			t = hlp.bust_attr(t, "\\author{")
			hlp.author = t.to_s
		elsif t.to_s.index("\\version{") != nil then
			t = hlp.bust_attr(t, "\\version{")
			hlp.revision = t.to_s
		end
	end
end


def ParseSections(hlp, txt)
	x = txt.length

	if x == 0 then return end
	x -= 1

	flag = true
	n = 0
	line = ""
	secname = ""
	section = false

	sect = nil

	while flag == true do
		if txt[n].index("\\section{") == 0 then
			if line.length > 0 then
				sect.lines << line.to_s
				line = ""
				sect = nil
			end

			section = true
			line = ""

			sect = HelpSection.new()

			t = hlp.bust_attr(txt[n], "\\section{")
			sect.name = t.to_s
			secname = t
			hlp.sections << sect
			n = n + 1
		end


		if section == true then
			if txt[n].chop.length == 0 then
				line = line.rstrip + "\n "
				sect.lines << line.to_s
				line = ""
			else
				q = txt[n].chop.to_s
				q = q.gsub("\\\\", "\n")
				q = q.gsub(/\t/,'        ')

				#if line.length > 0 then
				if (line[-1].to_s != "10" and line[-1].to_s != "13") and line.length > 0 then
					line += " " + q
				else
					line += "" + q
				end
			end
		end

		if n >= x then flag = false end
		n += 1
	end

	if line.length > 0 then
		sect.lines << line.to_s
		line = ""
		sect = nil
	end

	hlp.sections.sort!{|a,b|a.name <=> b.name}
end

@in_size = 0
@out_size = 0

if __FILE__ == $0
	if ARGV.length == 2
		if File.exists?(ARGV[0].to_s)

			hlp = HelpFile.new()

			txt = IO.readlines(ARGV[0].to_s)

			PutHeaders(hlp, txt)
			ParseSections(hlp, txt)

			ofp = File.open(ARGV[1].to_s, "wb")
			in_size, out_size = hlp.compile(ofp)
			ofp.close

			puts "Compiling #{ARGV[0].to_s} : in " + in_size.to_s + ", out " + out_size.to_s + ", saved " + (in_size - out_size).to_s + " bytes, shrunk to " + ((out_size.to_f / in_size.to_f) * 100).to_i.to_s + "% of original size."
		else
			puts "input file #{ARGV[1].to_s} does not exist"
		end
	else
		puts "helpcompiler input.txt output.hlp"
	end
end

