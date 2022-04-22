editors.map = {
    title = 'Map Editor',

    init = function(self)
        self.map = element(
            5, 15,                -- x, y
            scr_w - 10, 80,       -- w, h
            function(self)        -- render
                pix(
                    self.x, self.y, colors.secondary.bg,
                    { w = self.w, h = self.h }
                )
                editor_maprender(
                    1,
                    -self.x + self.offset.x,
                    -self.y + self.offset.y
                )

                -- TODO draw coordinates

                local col = 0x000000

                local x1 = self.x + self.w
                local y1 = self.y + self.h

                pix(0,  10,     col, { w = scr_w,      h = self.y - 10 })     -- top
                pix(0,  y1,     col, { w = scr_w,      h = scr_h - 10 - y1 }) -- bottom
                pix(0,  self.y, col, { w = self.x,     h = self.h })          -- left
                pix(x1, self.y, col, { w = scr_w - x1, h = self.h })          -- right
            end
        )

        self.map.mouse_down = function(self, x, y)
            local editor = editors.map
            local map = editor.map

            local xt = (x + map.offset.x) // 8
            local yt = (y + map.offset.y) // 8

            if xt >= 0 and xt < map_w and
               yt >= 0 and yt < map_h then
                if set_tile(xt, yt, editor.atlas.selected) then
                    editor.is_edited = true
                end
            end
        end

        self.map.offset = { x = 0, y = 0 }

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
                function(text) -- on_enter
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
                function(text) -- on_enter
                end
            ),
        }
    end,

    tick = function(self)
        -- tick map
        do
            local xm = 0
            local ym = 0

            if key(0) then ym = ym - 1 end
            if key(1) then xm = xm - 1 end
            if key(2) then ym = ym + 1 end
            if key(3) then xm = xm + 1 end

            local map = self.map

            map.offset.x = map.offset.x + xm * 2
            map.offset.y = map.offset.y + ym * 2
        end
    end,

    save = function(self)
        editor_save_map()
    end
}
