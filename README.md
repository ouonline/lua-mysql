`lua-mysql` is a MySQL client library for Lua. It is compatible with Lua 5.2.3(or above) and based on the [MySQL C API](http://dev.mysql.com/doc/refman/5.6/en/c-api-function-overview.html) provided by the official MySQL library.

Lua APIs
========

First you need to use `mysql = require('luamysql')` to import a table named `mysql`(or any other valid name).

* `client, errmsg = mysql.newclient(dbarg)`

    - Attempts to establish a connection to a MySQL server specified by `dbarg`. If successfully executed, a valid MySQL client `client`, plus a `nil` for `errmsg`, are returned; otherwise `client` will be `nil` and an error message `errmsg` is returned. See `luamysql-demo.lua` for more details about `dbarg`.

* `errmsg = client:ping()`

    - Checks whether the connection to the server is working. If the connection has gone down and auto-reconnect is enabled an attempt to reconnect is made. If the connection is down and auto-reconnect is disabled, an error message `errmsg` is returned.

* `errmsg = client:selectdb(dbname)`

    - Causes the database specified by `dbname` to become the default (current) database of the `client`. `nil` is returned if successfully executed, otherwise an error message `errmsg` is returned.

* `errmsg = client:setcharset(charset)`

    - Sets the default character set to be `charset` for the current connection. `nil` is returned if successfully executed, otherwise an error message `errmsg` is returned.

* `result, errmsg = client:escape(str)`

    - Converts the `str` to a legal SQL string that can be used in a SQL statement. If successfully executed, a `result` containing a valid string, and a `nil` for `errmsg`, are returned; otherwise the `result` will be `nil`, and an error message `errmsg` is returned.

* `result, errmsg = client:query(sqlstr)`

    - Executes a SQL statement `sqlstr`. If successfully executed, a `result` containing all information, and a `nil` for `errmsg`, are returned; otherwise the `result` will be `nil`, and the error message `errmsg` tells what happened.

* `size = result:size()`

    - Returns the number of record(s) in the `result`. `nil` is returned if error occurs.

* `fieldnamelist = result:fieldnamelist()`

    - Returns the fieldname list for the `result`. `nil` is returned if error occurs.

* `result:recordlist()`

    - Returns an iteration closure function which can be used in the `for ... in` form. `nil` is returned if error occurs. Each record returned by the iterator function is an array containing values corresponding to the fieldname list which is the result of `result:fieldnamelist()`.

See `luamysql-demo.lua` for more details.

C Library
=========

`lua-mysql` can also be integrated with C/C++ programs for executing Lua scripts. See `host.c` for how to import it into C/C++ environment.

FAQ
===

* If you try to run `luamysql-demo.lua` with the following command:

    > $ lua luamysql-demo.lua

  but encounter an error message like this:

    > lua dynamic libraries not enabled; check your Lua installation

  which means you need to re-compile Lua with extra arguments to enable loading dynamic libraries. For example, in Linux systems:

    > $ make posix MYCFLAGS=-DLUA\_USE\_DLOPEN MYLIBS=-ldl

* If there is an error message like this:

    > multiple Lua VMs detected

  you may re-compile the Lua interpreter with option `-Wl,-E`.

Enjoy it!
