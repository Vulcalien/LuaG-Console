local xc = scr_w / 2

editors.sprite = {
    title = "Sprite Editor",
    init = function()

    end,

    tick = function()

    end,

    render = function()
        -- sprite preview
        pix(xc - 25, 15, colors.primary.bg, 50, 50)
        -- TODO draw sprite

        -- scope selector
        pix(xc - 40, 30, colors.primary.bg, 10, 19)

        -- toolbox
        pix(xc - 64, 17, colors.primary.bg, 19, 46)

        -- color selector
        -- TODO consider making this a fixed width
        pix(xc + 30, 15, colors.primary.bg, scr_w - xc - 35, 50)


        editor_draw_atlas(scr_w / 2 - 64, scr_h - 10 - 69, 0, 8)
    end,

    buttons = {
        scope_big = {
            x = xc - 39, y = 31,
            icon = 48
        },
        scope_small = {
            x = xc - 39, y = 40,
            icon = 49
        },

        tool_pencil = {
            x = xc - 63, y = 18,
            icon = 32
        },
        tool_bucket = {
            x = xc - 63, y = 27,
            icon = 33
        },
        tool_pickup = {
            x = xc - 63, y = 36,
            icon = 34
        },
        undo = {
            x = xc - 63, y = 45,
            icon = 2
        },
        redo = {
            x = xc - 63, y = 54,
            icon = 3
        },
        tool_select = {
            x = xc - 54, y = 18,
            icon = 35
        }
    }
}
