#ifndef LUAKIT_EXTENSION_MSG_H
#define LUAKIT_EXTENSION_MSG_H

#include "common/msg.h"

int web_extension_connect(const gchar *socket_path);

void msg_recv_lua_require_module(const msg_lua_require_module_t *msg, guint length);
void msg_recv_lua_msg(const msg_lua_msg_t *msg, guint length);

#endif
