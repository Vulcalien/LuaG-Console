editors.sprite = {
    title = 'Sprite Editor',

    init = function(self)
        -- First declare 'tools', then define it. This allows the tool
        -- elements to reference 'tools'.
        local tools
        tools = {
            pencil = {
                act = function(self, editor, x, y)
                    local xt = editor.atlas.selected % 16
                    local yt = editor.atlas.selected // 16

                    editor_atlas_set_pixel(
                        xt * 8 + x,           -- x
                        yt * 8 + y,           -- y
                        editor.selected_color -- color
                    )
                end
            },
            bucket = {
                act = function(self, editor, x, y)
                    -- TODO fill
                end
            },
            pickup = {
                act = function(self, editor, x, y)
                    local xt = editor.atlas.selected % 16
                    local yt = editor.atlas.selected // 16

                    editor.selected_color = editor_atlas_get_pixel(
                        xt * 8 + x, -- x
                        yt * 8 + y  -- y
                    )

                    editor.selected_tool = tools.pencil
                end
            }
        }

        local xc = scr_w // 2

        self.canvas = canvas(
            self,    -- editor
            xc - 24, -- x
            17,      -- y
            6        -- scale
        )

        self.atlas = atlas(
            (scr_w - 128) / 2, -- x
            scr_h - 10 - 69,   -- y
            8                  -- rows
        )

        self.selected_tool = tools.pencil
        self.selected_color = 0xffffff -- TODO

        self.gui = {
            -- TOOLBOX
            -- toolbox background
            box(xc - 64, 17, 19, 46, colors.primary.bg),
            -- pencil
            button(
                xc - 63,       -- x
                18,            -- y
                32,            -- icon
                function(self) -- click_fn
                    local editor = editors.sprite
                    editor.selected_tool = tools.pencil
                end,
                function(self) -- highlight_fn
                    local editor = editors.sprite
                    return editor.selected_tool == tools.pencil
                end
            ),
            -- bucket
            button(
                xc - 63,       -- x
                27,            -- y
                33,            -- icon
                function(self) -- click_fn
                    local editor = editors.sprite
                    editor.selected_tool = tools.bucket
                end,
                function(self) -- highlight_fn
                    local editor = editors.sprite
                    return editor.selected_tool == tools.bucket
                end
            ),
            -- pickup
            button(
                xc - 63,       -- x
                36,            -- y
                34,            -- icon
                function(self) -- click_fn
                    local editor = editors.sprite
                    editor.selected_tool = tools.pickup
                end,
                function(self) -- highlight_fn
                    local editor = editors.sprite
                    return editor.selected_tool == tools.pickup
                end
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
            box(xc - 40, 27, 10, 28, colors.primary.bg),
            -- scope 1
            button(
                xc - 39,       -- x
                28,            -- y
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
            -- scope 2
            button(
                xc - 39,       -- x
                37,            -- y
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
            -- scope 3
            button(
                xc - 39,       -- x
                46,            -- y
                50,            -- icon
                function(self) -- click_fn
                    local editor = editors.sprite
                    editor.atlas:change_scope(3)
                end,
                function(self) -- highlight_fn
                    local editor = editors.sprite
                    return editor.atlas.scope == 3
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
