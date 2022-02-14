function atlas(editor, x, y, rows, select_fn)
    local result = element(
        x,      y,           -- x, y
        16 * 8, rows * 8,    -- w, h
        function(self, x, y) -- click
            xt = math.floor(x / 8)
            yt = math.floor(y / 8) -- TODO add scroll

            self.selected = xt + yt * 16
            if on_select then
                on_select()
            end
        end,
        function(self)       -- render
            -- TODO take scroll from editor
            editor_draw_atlas(x, y, 0, rows)
        end
    )

    result.selected = 0

    return result
end
