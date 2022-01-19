function init()
    colors = {
        primary = {
            bg = 0xdd4444,
            fg = 0x000000
        },
        secondary = {
            bg = 0xaa4444,
            fg = 0xeecccc
        },
        highlight = {
            fg = 0xffff55
        }
    }
end

function tick()
    render()
end

function render()
    clear(0x000000)

    do -- draw top bar
        pix(0, 0, colors.primary.bg, scr_w, 10)

        pix(1, 1, colors.secondary.bg, 8, 8)
        -- spr with modulation

        pix(12, 1, colors.secondary.bg, 8, 8)
        -- spr with modulation

        pix(23, 1, colors.secondary.bg, 8, 8)
        -- spr with modulation
    end

    do -- draw bottom bar
        pix(0, scr_h - 10, colors.primary.bg, scr_w, 10)
    end
end
