dnl @author Mooffie
dnl @license GPL
dnl @copyright Free Software Foundation, Inc.

AC_DEFUN([mc_WITH_LUA], [

    AC_ARG_WITH([lua],
        AS_HELP_STRING([--with-lua@<:@=PKG@:>@],
            [Enable Lua support]
        ),
        [:],
        [with_lua=no]        dnl In the future we'll automatically enable Lua if it exists on the system.
    )

    if test x"$with_lua" = xno; then

        with_lua=

    elif test -n "$LUA_CFLAGS" -o -n "$LUA_LIBS"; then

        with_lua=custom

    else

        dnl Packages don't have standard names so we have to try variations.
        dnl E.g., on Debian it's "lua5.1" but on FreeBSD it's "lua-5.1".
        tries51="lua5.1 lua-5.1 lua51"
        tries52="lua5.2 lua-5.2 lua52"
        tries53="lua5.3 lua-5.3 lua53"
        triesjit="luajit"

        case "$with_lua" in

            # Officially, we ask users to type "--with-lua=lua5.1" etc., but they're
            # likely to type "--with-lua=5.1" instead, so we accommodate for that as well.

            lua5.1|5.1) tries=$tries51  ;;
            lua5.2|5.2) tries=$tries52  ;;
            lua5.3|5.3) tries=$tries53  ;;
            luajit|jit) tries=$triesjit ;;
            yes|"")
                    dnl When no Lua package is explicitly specified:
                    dnl
                    dnl Currently, we try them in this order: LuaJIT, 5.3, 5.2, 5.1.
                    dnl The decision to put LuaJIT first is arbitrary. We may want to
                    dnl revisit this ordering issue in the future.
                    dnl
                    dnl The trailing "lua" (below) might be Lua 5.0, which is why it's
                    dnl the last try. If it's indeed 5.0, which we don't support, we'll
                    dnl detect this in the tests that are to follow.
                    tries="$triesjit $tries53 $tries52 $tries51 lua" ;;
            *)
                    dnl Search for a package named explicitly.
                    tries=$with_lua ;;
        esac

        with_lua=
        for try in $tries; do
            AC_MSG_NOTICE([looking for package '$try'...])
            PKG_CHECK_MODULES([LUA], [$try],
                [
                    with_lua=$try
                    break
                ],
                [:])
        done

        if test -z "$with_lua"; then
            AC_MSG_ERROR([I could not find your Lua engine. Instead of relying on pkg-config you may sepcify LUA_CFLAGS and LUA_LIBS explicitly. Please see instructions in src/lua/doc/guide/80-installation.md.])
        fi

    fi

    if test -n "$with_lua"; then
        AC_DEFINE(USE_LUA, 1, [Define to use Lua])
    fi

    if test -n "$with_lua"; then
        echo
        echo "  LUA ENGINE PACKAGE: '$with_lua'"
        echo "  RESULTS: $LUA_CFLAGS, $LUA_LIBS"
        echo
    else
        echo
        echo "  LUA ENGINE PACKAGE: NONE REQUESTED"
        echo
    fi
])


dnl
dnl This code is called after the Lua engine had been found.
dnl It checks the system's consistency and engine's features.
dnl
AC_DEFUN([mc_CHECK_LUA], [

    old_CPPFLAGS=$CPPFLAGS
    old_LIBS=$LIBS
    CPPFLAGS="$CPPFLAGS $LUA_CFLAGS"
    LIBS="$LIBS $LUA_LIBS"

    AC_CHECK_HEADER([lua.h], [], [AC_MSG_ERROR([I cannot find Lua's <lua.h> header. Something is probably amiss with the '-I' preprocessor switch.])])
    AC_CHECK_FUNC([lua_pushstring], [], [AC_MSG_ERROR([I cannot link against the Lua engine. Something is probably amiss with the LIBS variable. Examine 'config.log' for the exact error.])])

    AC_CHECK_FUNC([luaJIT_setmode],
        [
            lua_engine_title="LuaJIT"
            AC_DEFINE([HAVE_LUAJIT], [1], [Define if using LuaJIT.])
        ],
        [
            AC_CHECK_FUNC([lua_isinteger],
                [
                    lua_engine_title="Lua 5.3"
                    AC_CHECK_FUNC([lua_rotate], [:], [
                        dnl For our sample scripts we need a math.floor() that knows to return integers,
                        dnl and also math.tointger(). Older 'work' versions are problematic with these.
                        AC_MSG_ERROR([You are using an old beta version of Lua 5.3. It's outdated. I require the official version.])
                    ])
                ],
                [
                    AC_CHECK_FUNC([lua_callk], [lua_engine_title="Lua 5.2"], [
                        AC_CHECK_FUNC([luaL_newstate], [lua_engine_title="Lua 5.1"], [
                            AC_MSG_ERROR([You seem to be using an old version of Lua. I require at least Lua 5.1 (Lua 5.0 is NOT supported).])
                        ])
                    ])
                ]
            )
        ]
    )

    dnl Make SIZEOF_LUA_INTEGER and SIZEOF_LUA_NUMBER available to the C preprocessor.
    AC_CHECK_SIZEOF([lua_Integer], [], [
        #include <lua.h>
    ])
    AC_CHECK_SIZEOF([lua_Number], [], [
        #include <lua.h>
    ])

    dnl Feature tests. These are needed for our "capi" mini-library (in capi.{c,h}).
    AC_CHECK_FUNCS([lua_absindex luaL_getsubtable lua_rawlen luaL_testudata lua_getfenv luaL_setfuncs luaL_typerror])
    AC_CHECK_FUNCS([lua_pushunsigned luaL_checkunsigned luaL_checkint luaL_checklong luaL_optint luaL_optlong])
    AC_CHECK_FUNCS([luaL_setmetatable lua_isinteger])

    CPPFLAGS=$old_CPPFLAGS
    LIBS=$old_LIBS

    echo
    echo "  LUA ENGINE: $lua_engine_title"
    echo
])
