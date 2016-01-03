# Alaska

Alaska Workers is a distributed processing system built on top of [Malamute's service model](https://github.com/zeromq/malamute/blob/master/MALAMUTE.md). 

This model provides:

* Subscription by service name
* Filtering by subject
* Load balancing
* Reliable delivery

# Status

The Technology Readiness Level of this project is __experimental proof of concept__.

# Requirements

1. [CMake](https://cmake.org)
2. [Malamute](https://github.com/zeromq/malamute)
3. [Lua](http://www.lua.org/)
4. [LuaRocks](https://luarocks.org/)

Optionally, you may also want to install [LuaJIT](http://luajit.org/).

# Building from Source

Just run `./build.sh`.
If everything goes well you'll find the executable files in the 'bin/' folder.

# Running

First of all, make sure that an instance of Malamute is running, if not start one:

```
$> malamute
Malamute service/0.2.0
Copyright (c) 2014-15 the Contributors
This Software is provided under the MPLv2 License on an "as is" basis,
without warranty of any kind, either expressed, implied, or statutory.

I: 16-01-03 19:41:27 loading configuration from 'malamute.cfg'...
N: 16-01-03 19:41:27 server is using NULL security
```

Now you can run your worker processes:

```
$> ./alaska
Alaska Workers/0.0.1 (POC)
Copyright (c) 2016 the Contributors
This Software is provided under the MPLv2 License on an "as is" basis,
without warranty of any kind, either expressed, implied, or statutory.

I: 16-01-03 18:06:19 Loading configuration from 'alaska.cfg'
I: 16-01-03 18:06:19 Connected (AW::wildhorse.local::9352-BE39)
I: 16-01-03 18:06:19 Alaska worker ready (pool:4)
```

And send some service requests:

```
$> for run in {1..5}; do echo "{\"c\":${run}}" | ./alas; done
Service RQ
-Endpoint: tcp://127.0.0.1:9999
-Address: default
-Subject: dump
-Content:
{"c":1}
...
```

## Tips & Tricks

* The 'worker.scripts.dir' configuration key, defined in 'alaska.cfg', is relative to the directory where alaska is executed.
* The 'worker.workdir' configuration key is only active when running in background, i.e. as a daemon.