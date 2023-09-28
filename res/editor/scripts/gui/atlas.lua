function atlas(editor, x, y, rows)
    local result = element(
        x,      y,           -- x, y
        16 * 8, rows * 8,    -- w, h
        function(self)       -- render
            editor_spr(
                self.scrolled * 16, -- id
                x, y,               -- x, y
                1,                  -- scale
                16, rows            -- sw, wh
            )

            local selected_x = self.selected % 16
            local selected_y = self.selected // 16 - self.scrolled

            if selected_y < 0 or selected_y >= rows then
                return
            end

            local alpha
            if (ticks // 50) % 2 == 0 then
                alpha = 0xaa
            else
                alpha = 0xdd
            end

            spr(
                4,                       -- id
                self.x + selected_x * 8, -- x
                self.y + selected_y * 8, -- y
                { scale = 1, alpha = alpha } -- TODO scale
            )
        end
    )

    result.mouse_down = function(self, x, y)
        local xt = x // 8
        local yt = y // 8 + self.scrolled

        self.selected = xt + yt * 16
    end

    result.scroll = function(self, x, y, amount)
        self.scrolled = self.scrolled + amount

        if self.scrolled < 0 then
            self.scrolled = 0
        elseif self.scrolled + rows >= 16 then
            self.scrolled = 16 - rows
        end
    end

    result.selected = 0
    result.scrolled = 0

    return result
end
