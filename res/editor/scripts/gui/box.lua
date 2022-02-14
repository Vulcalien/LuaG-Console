function box(x, y, w, h, col)
    return element(
        x, y,      -- x, y
        w, h,      -- w, h
        function() -- render
            pix(x, y, col, w, h)
        end
    )
end
