#!/usr/bin/env bash -e

echo Building Alaska

cmake .
make
./bin/alaska --version

echo Installing Lua Libraries

luarocks install lua-cjson
luarocks install penlight
