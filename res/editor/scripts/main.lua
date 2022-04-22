function init()
    ticks = 0

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
        },
        textbox = {
            bg = 0xffffff,
            fg = 0x000000
        }
    }

    loadscript('gui/element.lua')
    loadscript('gui/box.lua')
    loadscript('gui/button.lua')
    loadscript('gui/textbox.lua')
    loadscript('gui/atlas.lua')

    gui = {
        -- top_bar
        box(0, 0, scr_w, 10, colors.primary.bg),

        -- bottom_bar
        element(
            0, scr_h - 10, -- x, y
            scr_w, 10,     -- w, h
            function(self) -- render
                pix(
                    self.x, self.y, colors.primary.bg,
                    { w = self.w, h = self.h }
                )

                write(
                    current_editor.title,        -- text
                    colors.secondary.fg,           -- color
                    self.x + 2,                  -- x
                    self.y + self.h - font_h - 1 -- y
                )
            end
        ),

        -- goto_term
        button(
            1, 1,      -- x, y
            1,         -- icon
            function() -- click_fn
                exit(0, 'Editor: exited to terminal')
            end
        ),
        -- map_editor
        button(
            12, 1,     -- x, y
            16,        -- icon
            function() -- click_fn
                current_editor = editors.map
            end
        ),
        -- sprite_editor
        button(
            23, 1,     -- x, y
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
                for key,editor in pairs(editors) do
                    if editor.is_edited then
                        print('Saving: ' .. key)
                        editor:save()
                        editor.is_edited = false
                    end
                end
            end,
            function()     -- highlight_fn
                for _,editor in pairs(editors) do
                    if editor.is_edited then
                        return true
                    end
                end
                return false
            end
        )
    }

    focused_element = nil

    do -- load editors
        editors = {}

        loadscript('editor/map.lua')
        loadscript('editor/sprite.lua')

        for _,editor in pairs(editors) do
            editor:init()
        end
    end

    current_editor = editors.map

    settransparent(0x000000)
end

function tick()
    if current_editor.tick then
        current_editor:tick()
    end

    do -- mouse actions
        local x, y = mouse_pos()

        if mouse(0) then
            gui_action('mouse_down', true, x, y)
        end

        if mouse_pressed(0) then
            gui_action('click', true, x, y)
        end

        if scroll() ~= 0 then
            gui_action('scroll', false, x, y, scroll())
        end
    end

    if focused_element and focused_element.tick_focus then
        focused_element:tick_focus()
    end

    ticks = ticks + 1
end

function gui_action(func_name, do_focus, x, y, ...)
    for _,element_list in pairs({ gui, current_editor.gui }) do
        for _,e in ipairs(element_list) do
            if x >= e.x and x < e.x + e.w and
               y >= e.y and y < e.y + e.h then
                if do_focus and e ~= focused_element then
                    if focused_element and focused_element.lose_focus then
                        focused_element:lose_focus()
                    end

                    if e.focus then
                        e:focus()
                    end
                    focused_element = e
                end

                if e[func_name] then
                    e[func_name](e, x - e.x, y - e.y, ...)
                    return
                end
            end
        end
    end
end

function render()
    clear(0x000000)

    for _,e in ipairs(current_editor.gui) do
        e:render()
    end

    for _,e in ipairs(gui) do
        e:render()
    end
end
