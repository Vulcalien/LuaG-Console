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
    loadscript("gui/box.lua")
    loadscript("gui/button.lua")
    loadscript("gui/atlas.lua")

    gui = {
        -- goto_term
        button(
            1,         -- x
            1,         -- y
            1,         -- icon
            function() -- click_fn
                -- TODO
            end
        ),
        -- map_editor
        button(
            12,        -- x
            1,         -- y
            16,        -- icon
            function() -- click_fn
                current_editor = editors.map
            end
        ),
        -- sprite_editor
        button(
            23,        -- x
            1,         -- y
            17,        -- icon
            function() -- click_fn
                current_editor = editors.sprite
            end
        ),
        -- save
        button(
            scr_w - 1 - 8, -- x
            1,             -- y
            0,             -- icon
            function()     -- click_fn
                -- TODO save click
            end,
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

    for _,editor in pairs(editors) do
        editor:init()
    end

    current_editor = editors.map

    settransparent(0x000000)
end

function tick()
    -- GUI click action
    if mouse_pressed(0) then
        local x, y = mouse_pos()

        for _,element_list in pairs({ gui, current_editor.gui }) do
            for _,e in ipairs(element_list) do
                if x >= e.x and x < e.x + e.w and
                   y >= e.y and y < e.y + e.h then
                    if e.click then
                        e:click(x - e.x, y - e.y)
                    end
                    break
                end
            end
        end
    end

    render()
end

function render()
    clear(0x000000)

    for _,e in ipairs(current_editor.gui) do
        e:render()
    end

    -- draw top bar
    pix(0, 0, colors.primary.bg, scr_w, 10)

    -- draw bottom bar
    pix(0, scr_h - 10, colors.primary.bg, scr_w, 10)

    write(current_editor.title, colors.secondary.fg, 2, scr_h - font_h - 1)

    -- render GUI elements
    for _,e in ipairs(gui) do
        e:render()
    end
end
