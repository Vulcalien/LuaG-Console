function init()
    clear(0xff0000)
    loadscript("test.lua")
    write("asd", 0x000000, 1, 1)
    pix(100, 100, 0x00ffff, 5, 2)
end

function tick()
    a = loadscript()
    print(a)
end
