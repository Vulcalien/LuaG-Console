function map_element(editor, x, y, w, h)
    local result = element(
        x, y,          -- x, y
        w, h,          -- w, h
        function(self) -- render
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

    result.tick = function(self)
        local xm = 0
        local ym = 0

        if key(0) then ym = ym - 1 end
        if key(1) then xm = xm - 1 end
        if key(2) then ym = ym + 1 end
        if key(3) then xm = xm + 1 end

        self.offset.x = self.offset.x + xm * 2
        self.offset.y = self.offset.y + ym * 2
    end

    result.mouse_down = function(self, x, y)
        local xt = (x + self.offset.x) // 8
        local yt = (y + self.offset.y) // 8

        if xt >= 0 and xt < map_w and
           yt >= 0 and yt < map_h then
            if set_tile(xt, yt, editor.atlas.selected) then
                editor.is_edited = true
            end
        end
    end

    result.offset = { x = 0, y = 0 }

    return result
end
