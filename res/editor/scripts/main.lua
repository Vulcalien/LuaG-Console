function init()
    editor_load_files()

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

    loadscript("gui/element.lua")
    loadscript("gui/button.lua")

    gui = {
        goto_term = button(
            1, -- x
            1, -- y
            1  -- icon
        ),
        map_editor = button(
            12, -- x
            1,  -- y
            16  -- icon
        ),
        sprite_editor = button(
            23, -- x
            1,  -- y
            17  -- icon
        ),
        save = button(
            scr_w - 1 - 8, -- x
            1,             -- y
            0,             -- icon
            function()     -- highlight_fn
                -- TODO highlight "save" icon
                return false
            end
        )
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

    current_editor.render()

    -- draw top bar
    pix(0, 0, colors.primary.bg, scr_w, 10)

    -- draw bottom bar
    pix(0, scr_h - 10, colors.primary.bg, scr_w, 10)

    write(current_editor.title, colors.secondary.fg, 2, scr_h - font_h - 1)

    for _,element in pairs(gui) do
        element:render()
    end

    for _,element in pairs(current_editor.gui) do
        element:render()
    end
end
