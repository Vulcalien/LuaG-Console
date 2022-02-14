function atlas(editor, x, y, rows, select_fn)
    local result = element(
        x,      y,           -- x, y
        16 * 8, rows * 8,    -- w, h
        function(self)       -- render
            -- TODO consider scroll (instead of 0)
            editor_draw_atlas(x, y, 0, rows)

            local selected_x = self.selected % 16
            local selected_y = math.floor(self.selected / 16)

            -- TODO
            --local alpha
            --if math.floor(ticks / 50) % 2 == 0 then
                --alpha = 0xaa
            --else
                --alpha = 0xdd
            --end

            spr(
                4,                       -- id
                self.x + selected_x * 8, -- x
                self.y + selected_y * 8, -- y
                { scale = 1 } -- TODO scale
            )
        end,
        function(self, x, y) -- click
            local xt = math.floor(x / 8)
            local yt = math.floor(y / 8) -- TODO add scroll

            self.selected = xt + yt * 16
            if on_select then
                on_select()
            end
        end
    )

    result.selected = 0

    return result
end
