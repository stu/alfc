#!/usr/bin/ruby

#
# Hacky ruby script to compile the help text into our help file format.
#

require 'zlib'

def BustAttr(s, a)
	b = s.to_s.index(a)
	if b == nil then return end

	if b >= 0 then
		x = s[(b + a.length) .. (b+a.length) + s[(b+a.length)..-1].index("}") - 1].to_s
	end
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

def PutHeaders(txt, ofp)
	txt.each do |t|
		if t.to_s.index("\\title{") != nil then
			t = BustAttr(t, "\\title{")
			bdump(t, 0x01, ofp)
		elsif t.to_s.index("\\author{") != nil then
			t = BustAttr(t, "\\author{")
			bdump(t, 0x02, ofp)
		elsif t.to_s.index("\\version{") != nil then
			t = BustAttr(t, "\\version{")
			bdump(t, 0x03, ofp)
		end
	end
end

def outline(x, ofp)
	xx = x.split("\n")

	xx.each do |xxx|
		#puts "line: #{xx}"
		q = xxx.rstrip
		if q[-1].to_s == "\n" then q = q.chop end
		bdump_gz(q, 0x05, ofp)
	end
	return ""
end

def ParseSections(txt, ofp)
	x = txt.length

	if x == 0 then return end
	x -= 1

	flag = true
	n = 0
	line = ""
	secname = ""
	section = false

	while flag == true do
		if txt[n].index("\\section{") == 0 then
			if line.length > 0 then
				line = outline(line, ofp)
			end

			section = true
			line = ""

			t = BustAttr(txt[n], "\\section{")
			puts "Section #{t}"
			secname = t
			bdump(t, 0x04, ofp)

			n = n + 1
		end


		if section == true then
			if txt[n].chop.length == 0 then
				line = line.rstrip + "\n "
				line = outline(line, ofp)
			else
				q = txt[n].chop.to_s
				q = q.gsub("\\\\", "\n")
				q = q.gsub(/\t/,'        ')

				#if line.length > 0 then
				if line[-1].to_s != "10" and line.length > 0 then
					q = q.lstrip
				end

				line += " " + q
			end
		end

		if n >= x then flag = false end
		n += 1
	end

	if line.length > 0 then
		line = outline(line, ofp)
	end

end

@in_size = 0
@out_size = 0

if __FILE__ == $0
	if ARGV.length == 2
		if File.exists?(ARGV[0].to_s)
			txt = IO.readlines(ARGV[0].to_s)

			ofp = File.open(ARGV[1].to_s, "wb")
			ofp.putc 0x28
			ofp.putc 0x4C

			PutHeaders(txt, ofp)
			ParseSections(txt, ofp)

			ofp.close

			puts "Compiling #{ARGV[0].to_s} : in " + @in_size.to_s + ", out " + @out_size.to_s + ", saved " + (@in_size - @out_size).to_s + " bytes, shrunk to " + ((@out_size.to_f / @in_size.to_f) * 100).to_i.to_s + "% of original size."

		else
			puts "input file #{ARGV[1].to_s} does not exist"
		end
	else
		puts "helpcompiler input.txt output.hlp"
	end
end

