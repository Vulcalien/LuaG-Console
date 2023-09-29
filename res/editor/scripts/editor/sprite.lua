local xc = scr_w / 2

editors.sprite = {
    title = 'Sprite Editor',

    init = function(self)
        self.canvas = element(
            xc - 24, 17,   -- x, y
            48,      48,   -- w, h
            function(self) -- render
                local editor = editors.sprite

                editor_spr(
                    editor.atlas.selected, -- id
                    self.x,                -- x
                    self.y,                -- y
                    6,                     -- scale
                    1,                     -- sw
                    1                      -- sh
                )
            end
        )

        self.canvas.mouse_down = function(self, x, y)
            local editor = editors.sprite

            -- TODO draw
        end

        self.atlas = atlas(
            self,              -- editor
            (scr_w - 128) / 2, -- x
            scr_h - 10 - 69,   -- y
            8                  -- rows
        )

        self.gui = {
            -- TOOLBOX
            -- toolbox background
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
                xc - 39,       -- x
                31,            -- y
                48,            -- icon
                function(self) -- click_fn
                    local editor = editors.sprite
                    editor.atlas:change_scope(1)
                end,
                function(self) -- highlight_fn
                    local editor = editors.sprite
                    return editor.atlas.scope == 1
                end
            ),
            -- scope_small
            button(
                xc - 39,       -- x
                40,            -- y
                49,            -- icon
                function(self) -- click_fn
                    local editor = editors.sprite
                    editor.atlas:change_scope(2)
                end,
                function(self) -- highlight_fn
                    local editor = editors.sprite
                    return editor.atlas.scope == 2
                end
            ),

            -- color selector
            box(xc + 30, 15, scr_w - xc - 35, 50, colors.primary.bg),

            -- canvas background
            box(xc - 26, 15, 52, 52, colors.primary.bg),

            -- canvas
            self.canvas,

            -- atlas
            self.atlas
        }
    end,

    save = function(self)

    end
}
