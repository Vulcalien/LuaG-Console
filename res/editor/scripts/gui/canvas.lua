function canvas(editor, x, y, scale)
    local result = element(
        x,         y,         -- x, y
        8 * scale, 8 * scale, -- w, h
        function(self)        -- render
            local atlas = editor.atlas

            editor_spr(
                atlas.selected,       -- id
                self.x,               -- x
                self.y,               -- y
                scale // atlas.scope, -- scale
                atlas.scope,          -- sw
                atlas.scope           -- sh
            )
        end
    )

    result.mouse_down = function(self, x, y)
        local atlas = editor.atlas

        local xpix = x * atlas.scope // scale
        local ypix = y * atlas.scope // scale
    end

    return result
end
