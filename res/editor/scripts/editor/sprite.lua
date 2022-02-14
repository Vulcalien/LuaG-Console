local xc = scr_w / 2

editors.sprite = {
    title = "Sprite Editor",

    init = function(self)
        self.gui = {
            -- TOOLBOX
            -- toolbox_background
            box(xc - 64, 17, 19, 46, colors.primary.bg),
            -- pencil
            button(
                xc - 63, -- x
                18,      -- y
                32       -- icon
            ),
            -- bucket
            button(
                xc - 63, -- x
                27,      -- y
                33       -- icon
            ),
            -- pickup
            button(
                xc - 63, -- x
                36,      -- y
                34       -- icon
            ),
            -- undo
            button(
                xc - 63, -- x
                45,      -- y
                2        -- icon
            ),
            -- redo
            button(
                xc - 63, -- x
                54,      -- y
                3        -- icon
            ),
            -- select
            button(
                xc - 54, -- x
                18,      -- y
                35       -- icon
            ),

            -- SCOPE SELECTORS
            -- scope_background
            box(xc - 40, 30, 10, 19, colors.primary.bg),
            -- scope_big
            button(
                xc - 39, -- x
                31,      -- y
                48       -- icon
            ),
            -- scope_small
            button(
                xc - 39, -- x
                40,      -- y
                49       -- icon
            ),

            -- color selector
            -- TODO consider making this a fixed width
            box(xc + 30, 15, scr_w - xc - 35, 50, colors.primary.bg), -- TODO this is temporary

            -- sprite preview
            box(xc - 25, 15, 50, 50, colors.primary.bg), -- TODO this is temporary

            -- atlas
            atlas(
                self,              -- editor
                (scr_w - 128) / 2, -- x
                scr_h - 10 - 69,   -- y
                8,                 -- rows
                function()         -- on_select
                    -- TODO
                end
            )
        }
    end,

    tick = function(self)

    end
}
