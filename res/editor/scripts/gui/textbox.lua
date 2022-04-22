function textbox(x, y, size_limit, input_type, on_enter)
    local result = element(
        x, y, size_limit * (font_w + 1) + 1, font_h + 1,
        function(self) -- render
            pix(self.x, self.y, 0xffffff, { w = self.w, h = self.h })
            write(self.text, 0x000000, self.x + 1, self.y + 1)
        end
    )

    result.tick_focus = function(self)
        local input = editor_get_text()

        for i=1,string.len(input) do
            local char = string.sub(input, i, i)

            if char == '\n' then
                on_enter(self.text)

                self.lose_focus()
                focused_element = nil
            elseif char == '\b' then
                self.text = string.sub(str, 1, -2) -- TODO probably wrong + check bounds
            else
                if string.len(self.text) < size_limit then
                    if (input_type == 'dec' and tonumber(char, 10)) or
                       (input_type == 'hex' and tonumber(char, 16)) then
                        self.text = self.text .. char
                    end
                end
            end
        end
    end

    result.focus = function()
        editor_set_text_mode(true)
    end

    result.lose_focus = function()
        editor_set_text_mode(false)
    end

    result.text = ''

    return result
end
