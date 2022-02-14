local function btn_render(self)
    pix(self.x, self.y, colors.secondary.bg, self.w, self.h)

    local col
    if self.highlight_fn and self:highlight_fn() then
        col = colors.highlight.fg
    else
        col = colors.secondary.fg
    end

    spr(
        self.icon,      -- id
        self.x, self.y, -- x, y
        1,              -- scale
        1, 1,           -- sw, sh,
        0,              -- rot
        false, false,   -- h_flip, v_flip
        col             -- color_mod
    )
end

function button(x, y, icon, click_fn, highlight_fn)
    local result = element(
        x, y,       -- x, y
        8, 8,       -- w, h
        btn_render, -- render
        click_fn    -- click
    )

    result.icon = icon
    result.highlight_fn = highlight_fn

    return result
end
