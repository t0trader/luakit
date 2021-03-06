#include "extension/clib/extension.h"
#include "extension/clib/page.h"

typedef struct _extension_t {
    LUA_OBJECT_HEADER
} extension_t;

lua_class_t extension_class;
gint extension_ref;
GPtrArray *queued_emissions;

LUA_OBJECT_FUNCS(extension_class, extension_t, extension);

static void
page_created_cb(WebKitWebExtension *UNUSED(extension), WebKitWebPage *web_page, lua_State *L)
{
    /* Since web modules are loaded after the first web page is created, signal
     * handlers bound to the page-created signal will not be called for the
     * first web page... unless we queue the signal and emit it later, when the
     * configuration file (and therefore all modules) has been loaded */
    if (queued_emissions) {
        g_ptr_array_add(queued_emissions, web_page);
        return;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, extension_ref);
    luaH_checkudata(L, -1, &extension_class);
    luaH_page_from_web_page(L, web_page);
    luaH_object_emit_signal(L, -2, "page-created", 1, 0);
    lua_pop(L, 1);
}

static int
luaH_extension_new(lua_State *L)
{
    lua_newtable(L);
    luaH_class_new(L, &extension_class);
    lua_remove(L, -2);
    return 1;
}

void
extension_class_setup(lua_State *L, WebKitWebExtension *extension)
{
    static const struct luaL_reg extension_methods[] =
    {
        LUA_CLASS_METHODS(extension)
        { NULL, NULL }
    };

    static const struct luaL_reg extension_meta[] =
    {
        LUA_OBJECT_META(extension)
        { "__gc", luaH_object_gc },
        { NULL, NULL }
    };

    luaH_class_setup(L, &extension_class, "extension",
            (lua_class_allocator_t) extension_new,
            NULL, NULL,
            extension_methods, extension_meta);

    queued_emissions = g_ptr_array_sized_new(1);
    luaH_extension_new(L);
    lua_setglobal(L, "extension");
    lua_getglobal(L, "extension");
    extension_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    g_signal_connect(extension, "page-created", G_CALLBACK(page_created_cb), L);
}

void
extension_class_emit_pending_signals(lua_State *L)
{
    GPtrArray *tmp = queued_emissions;
    queued_emissions = NULL;

    for (unsigned i = 0; i < tmp->len; i++) {
        WebKitWebPage *web_page = g_ptr_array_index(tmp, i);
        page_created_cb(NULL, web_page, L);
    }

    g_ptr_array_free(tmp, TRUE);
}

// vim: ft=c:et:sw=4:ts=8:sts=4:tw=80
