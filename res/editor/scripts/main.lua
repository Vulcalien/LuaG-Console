function init()
    colors = {
        primary = {
            bg = 0xdd4444,
            fg = 0x000000
        },
        secondary = {
            bg = 0xaa4444,
            fg = 0xeecccc
        },
        highlight = {
            fg = 0xffff55
        }
    }

    -- this only includes buttons in the core GUI
    buttons = {
        goto_term = {
            x = 1, y = 1,
            icon = 1,
            is_highlighted = function()
                return false
            end
        },
        map_editor = {
            x = 12, y = 1,
            icon = 16,
            is_highlighted = function()
                return false
            end
        },
        sprite_editor = {
            x = 23, y = 1,
            icon = 17,
            is_highlighted = function()
                return false
            end
        },
        save = {
            x = scr_w - 1 - 8, y = 1,
            icon = 0,
            is_highlighted = function()
                -- TODO highlight "save" icon
                return false
            end
        }
    }

    -- load editors
    editors = {}

    loadscript("editor/map.lua")
    loadscript("editor/sprite.lua")

    editors.map.init()
    editors.sprite.init()

    current_editor = editors.map

    settransparent(0x000000)
end

function tick()
    current_editor.tick()

    render()
end

function render()
    clear(0x000000)

    -- draw top bar
    pix(0, 0, colors.primary.bg, scr_w, 10)

    -- draw bottom bar
    pix(0, scr_h - 10, colors.primary.bg, scr_w, 10)

    write(current_editor.title, colors.secondary.fg, 1, scr_h - font_h - 1)

    -- draw buttons
    for _,btn in pairs(buttons) do
        pix(btn.x, btn.y, colors.secondary.bg, 8, 8)

        local col
        if btn.is_highlighted() then
            col = colors.highlight.fg
        else
            col = colors.secondary.fg
        end

        spr(
            btn.icon,       -- id
            btn.x, btn.y,   -- x, y
            1,              -- scale
            1, 1,           -- sw, sh,
            0,              -- rot
            false, false,   -- h_flip, v_flip
            col             -- color_mod
        )
    end

    current_editor.render()
end