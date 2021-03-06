local ui_process = ui_process
local page = page
local type = type
local assert = assert

module("follow_selected_webmodule")

local ui = ui_process()

-- Return selected uri or first uri in selection
local return_selected = [=[
(function(document) {
    var selection = window.getSelection(),
        container = document.createElement('div'),
        range, elements, i = 0;

    if (selection.toString() !== "") {
        range = selection.getRangeAt(0);
        // Check for links contained within the selection
        container.appendChild(range.cloneContents());

        var elements = container.getElementsByTagName('a'),
            len = elements.length, i = 0, href;

        for (; i < len;)
            if ((href = elements[i++].href))
                return href;

        // Check for links which contain the selection
        container = range.startContainer;
        while (container !== document) {
            if ((href = container.href))
                return href;
            container = container.parentNode;
        }
    }
    // Check for active links
    var element = document.activeElement;
    return element.src || element.href;
})(document);
]=]

ui:add_signal("follow_selected", function(_, action, view_id)
    local p = page(view_id)
    local uri = p:eval_js(return_selected)
    if not uri then return end
    assert(type(uri) == "string")
    ui:emit_signal(action, uri, view_id)
end)
