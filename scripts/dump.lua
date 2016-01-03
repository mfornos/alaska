-----------------------------------------------------
-- Dump Service
-----------------------------------------------------
-- Parses a JSON representation of the Alaska context global
-- variable and dumps the resulting table using Penlight
-----------------------------------------------------

local s = string.rep('-', 80)
print(s..'\nContext dump from Lua:')

local ctx = require 'cjson'.decode(alaskaContext)
require 'pl.pretty'.dump(ctx)

print(s)
