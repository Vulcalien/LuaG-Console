function init()
    clear(0xff0000)
    write("asd", 0x000000, 1, 1)
    pix(100, 100, 0x00ffff, 5, 2)

    loadscript("../scripttest.lua")
    foo()
end

function tick()
    a = loadscript()
    print(a)
end
