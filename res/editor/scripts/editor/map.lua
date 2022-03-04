editors.map = {
    title = "Map Editor",

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

        self.map.offset = { x = 0, y = 0 }

        self.atlas = atlas(
            self,              -- editor
            (scr_w - 128) / 2, -- x
            scr_h - 10 - 37,   -- y
            4                  -- rows
        )

        self.gui = {
            self.map,
            self.atlas
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

            if mouse(0) then
                local x, y = mouse_pos()

                if x >= map.x and x < map.x + map.w and
                   y >= map.y and y < map.y + map.h then
                    x = x - map.x
                    y = y - map.y

                    local xt = math.floor((x + map.offset.x) / 8)
                    local yt = math.floor((y + map.offset.y) / 8)

                    if xt >= 0 and xt < map_w and
                       yt >= 0 and yt < map_h then
                        set_tile(xt, yt, self.atlas.selected)
                        is_edited = true
                    end
               end
            end
        end
    end
}
