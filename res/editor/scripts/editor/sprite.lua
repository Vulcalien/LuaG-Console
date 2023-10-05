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

                    editor.is_edited = true
                end
            },
            bucket = {
                act = function(self, editor, x, y)
                    local xt = editor.atlas.selected % 16
                    local yt = editor.atlas.selected // 16

                    -- fill bounds
                    local x0 = xt * 8
                    local y0 = yt * 8
                    local x1 = (xt + editor.atlas.scope) * 8 - 1
                    local y1 = (yt + editor.atlas.scope) * 8 - 1

                    editor_atlas_fill(
                        x0 + x, y0 + y,        -- x, y
                        editor.selected_color, -- color
                        x0, y0, x1, y1         -- x0, y0, x1, y1
                    )

                    editor.is_edited = true
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
            -- scope background
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

            -- palette background
            box(xc + 30, 15, scr_w - xc - 35, 52, colors.primary.bg),

            -- canvas background
            box(xc - 26, 15, 52, 52, colors.primary.bg),

            -- canvas
            self.canvas,

            -- atlas
            self.atlas
        }

        -- DEBUG
        self.palette = {
            0xffffff, 0xff0000, 0xffff00, 0xffffff,
            0x000000, 0x0000ff, 0x0000ff, 0x000077,
            0xff0000, 0xff0000, 0x770000, 0x777777,
            0x111111, 0x222222, 0x333333, 0x444444,
        }

        -- Add palette color selectors to GUI
        for i=0,15 do
            local xt = i %  4
            local yt = i // 4

            -- add selector background
            table.insert(self.gui, box(
                xc + 31 + xt * 11,  -- x
                16      + yt * 11,  -- y
                10, 10,             -- w, h
                colors.secondary.bg -- col
            ))

            -- add selector
            local selector = element(
                xc + 32 + xt * 11, -- x
                17      + yt * 11, -- y
                8, 8,              -- w, h
                function(self)     -- render
                    local editor = editors.sprite
                    local col = editor.palette[1 + i]

                    pix(self.x, self.y, col, { w = self.w, h = self.h })

                    if editor.selected_color == col then
                        spr(5, self.x, self.y)
                    end
                end
            )
            selector.mouse_down = function(self)
                local editor = editors.sprite
                local col = editor.palette[1 + i]

                editor.selected_color = col
            end
            table.insert(self.gui, selector)
        end
    end,

    save = function(self)
        editor_save_atlas()
    end
}
