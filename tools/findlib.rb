
def FindLib(libs, paths)
    cwd = Dir.getwd

    while paths.length > 0
        p = paths[0]

        paths[0] = nil
        paths.delete_at(0)

        ok = true
        begin
            Dir.chdir("#{p}")
        rescue
            ok = false
        end


        if ok == true
            x = Dir["*"]
            x.each do |f|
                if f == "." or f == ".."
                    break
                end

                begin
                    link = File.readlink("#{p}")
                    if link == "." or link == ".."
                        break
                    end
                rescue NotImplementedError
                    #catch specific (seems win32 throws and rescue all wont catch it)
                rescue
                    # all others
                end

                if  File.directory?("#{p}/#{f}")
                    paths.insert(0, "#{p}/#{f}")
                else
                    libs.each do |l|
                        if f == "lib#{l}.a"
							Dir.chdir(cwd)
                            return "#{p}/#{f}"
                        elsif f == "lib#{l}.lib"
							Dir.chdir(cwd)
                            return "#{p}/#{f}"
                        end
                    end
                end
            end
        end

    end

    Dir.chdir(cwd)
    return "FAIL"
end


def FindInclude(incs, paths)
    cwd = Dir.getwd

    while paths.length > 0
        p = paths[0]

        paths[0] = nil
        paths.delete_at(0)

        ok = true
        begin
            Dir.chdir("#{p}")
        rescue
            ok = false
        end


        if ok == true
            x = Dir["*"]
            x.each do |f|
                if f == "." or f == ".."
                    break
                end

                begin
                    link = File.readlink("#{p}")
                    if link == "." or link == ".."
                        break
                    end
                rescue NotImplementedError
                    #catch specific (seems win32 throws and rescue all wont catch it)
                rescue
                    # all others
                end

                if  File.directory?("#{p}/#{f}")
                    paths.insert(0, "#{p}/#{f}")
                else
                    incs.each do |i|
                        if f == "#{i}"
							Dir.chdir(cwd)
                            return "#{p}"
                        end
                    end
                end
            end
        end

    end

    Dir.chdir(cwd)
    return "FAIL"
end
