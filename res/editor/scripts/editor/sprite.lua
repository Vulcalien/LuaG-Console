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

    gui = {
        scope_big = button(
            xc - 39, -- x
            31,      -- y
            48       -- icon
       ),
        scope_small = button(
            xc - 39, -- x
            40,      -- y
            49       -- icon
       ),

        tool_pencil = button(
            xc - 63, -- x
            18,      -- y
            32       -- icon
       ),
        tool_bucket = button(
            xc - 63, -- x
            27,      -- y
            33       -- icon
       ),
        tool_pickup = button(
            xc - 63, -- x
            36,      -- y
            34       -- icon
       ),
        undo = button(
            xc - 63, -- x
            45,      -- y
            2        -- icon
       ),
        redo = button(
            xc - 63, -- x
            54,      -- y
            3        -- icon
       ),
        tool_select = button(
            xc - 54, -- x
            18,      -- y
            35       -- icon
       )
    }
}
