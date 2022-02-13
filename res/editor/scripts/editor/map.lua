editors.map = {
    title = "Map Editor",

    init = function(self)
        self.gui = {
            -- map preview
            element(
                5, 15,          -- x, y
                scr_w - 10, 80, -- w, h
                function(self)  -- click
                    -- TODO
                end,
                function(self)  -- render
                    pix(self.x, self.y, colors.secondary.bg, self.w, self.h)
                    editor_maprender(1, -self.x, -self.y)

                    local col = 0x000000

                    local x1 = self.x + self.w
                    local y1 = self.y + self.h

                    pix(0,  10,     col, scr_w,      self.y - 10)     -- top
                    pix(0,  y1,     col, scr_w,      scr_h - 10 - y1) -- bottom
                    pix(0,  self.y, col, self.x,     self.h)          -- left
                    pix(x1, self.y, col, scr_w - x1, self.h)          -- right
                end
            ),
            -- atlas
            atlas(
                self,              -- editor
                (scr_w - 128) / 2, -- x
                scr_h - 10 - 37,   -- y
                4,                 -- rows
                function(id)       -- select_fn
                    -- TODO
                end
            )
        }
    end
}
