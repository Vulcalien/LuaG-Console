editors.sprite = {
    title = "Sprite Editor",
    init = function()

    end,

    tick = function()

    end,

    render = function()
        xc = scr_w / 2

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
    end,

    buttons = {}
}
