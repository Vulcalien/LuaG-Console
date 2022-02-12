editors.map = {
    title = "Map Editor",
    init = function()

    end,

    tick = function()

    end,

    render = function()
        -- draw map and clear the surrounding area
        pix(5, 15, colors.secondary.bg, scr_w - 10, 80)
        editor_maprender(1, -5, -15)

        pix(0,         10, 0x000000, scr_w, 5)               --top
        pix(0,         95, 0x000000, scr_w, scr_h - 20 - 85) --bottom
        pix(0,         15, 0x000000, 5,     80)              --left
        pix(scr_w - 5, 15, 0x000000, 5,     80)              --right

        -- TODO draw size selectors

        editor_draw_atlas(scr_w / 2 - 64, scr_h - 10 - 37, 0, 4)
    end,

    gui = {}
}
