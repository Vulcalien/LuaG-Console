function init()
    tick_count = 0

    clear(0xff0000)
    write("asd", 0x000000, 1, 1)
    pix(100, 100, 0x00ffff, 5, 2)

    loadscript("test.lua")
    foo()
end

function tick()
    tick_count = tick_count + 1

    --clear(0xff0000)
    spr(
        0, -- id
        1, 1, -- x, y
        10, -- scale
        1, 1, -- sw, sh
        tick_count // 10, -- rot
        false, false);
end
