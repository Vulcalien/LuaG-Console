editors.map = {
    title = 'Map Editor',

    init = function(self)
        self.map = map_element(
            self,           -- editor
            5,          15, -- x, y
            scr_w - 10, 80  -- w, h
        )

        self.atlas = atlas(
            self,              -- editor
            (scr_w - 128) / 2, -- x
            scr_h - 10 - 37,   -- y
            4                  -- rows
        )

        self.gui = {
            self.map,
            self.atlas,

            -- size selector background
            box(
                16, 99,
                self.atlas.w, font_h + 3,
                colors.primary.bg
            ),

            -- width label
            element(
                17, 101,
                29, font_h,
                function(self) --render
                    write('width', colors.primary.fg, self.x, self.y)
                end
            ),
            -- width textbox
            textbox(
                47, 100,
                4, 'dec',
                map_w,
                function(self, new_width) -- on_enter
                    if new_width == map_w then
                        return
                    end

                    if new_width == '' then
                        new_width = 0
                    end

                    local editor = editors.map
                    editor_update_map_size(
                        new_width, -1,
                        editor.atlas.selected
                    )

                    self.text = map_w
                    editor.is_edited = true
                end
            ),

            -- height label
            element(
                82, 101,
                35, font_h,
                function(self) --render
                    write('height', colors.primary.fg, self.x, self.y)
                end
            ),
            -- height textbox
            textbox(
                118, 100,
                4, 'dec',
                map_h,
                function(self, new_height) -- on_enter
                    if new_height == map_h then
                        return
                    end

                    if new_height == '' then
                        new_height = 0
                    end

                    local editor = editors.map
                    editor_update_map_size(
                        -1, new_height,
                        editor.atlas.selected
                    )

                    self.text = map_h
                    editor.is_edited = true
                end
            ),
        }
    end,

    tick = function(self)
        self.map:tick()
    end,

    save = function(self)
        editor_save_map()
    end
}
