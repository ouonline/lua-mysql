local mysql = require('luamysql')

dbhost = "127.0.0.1"
dbport = 3306
dbuser = "ouonline"
dbpassword = "foobar"
dbname = "test"

-------------------------------------------------------------------------------

client, errmsg = mysql.newclient(dbhost, dbport, dbuser, dbpassword)
if errmsg ~= nil then
    io.write("connect to mysql error: ", errmsg, "\n")
    return
end

errmsg = client:selectdb(dbname)
if errmsg ~= nil then
    io.write("selectdb error: ", errmsg, "\n")
    return
end

--[[
the above 2 function calls can be merged into one:
client, errmsg = mysql.newclient(host, port [, user [, password [, db] ] ])
--]]

errmsg = client:setcharset("utf8")
if errmsg ~= nil then
    io.write("setcharset error: ", errmsg, "\n")
    return
end

errmsg = client:ping()
if errmsg ~= nil then
    io.write("ping: ", errmsg, "\n")
    return
end

result, errmsg = client:escape("'ouonline'")
if errmsg ~= nil then
    io.write("escape error: ", errmsg, "\n")
    return
end
io.write("excape string -> ", result, "\n")

result, errmsg = client:query("select * from test")
if errmsg ~= nil then
    io.write("query error: ", errmsg, "\n")
    return
end

io.write("result count = ", result:count(), "\n")

fieldnamelist = result:fieldnamelist()
if fieldnamelist == nil then
    io.write("get fieldnamelist error.\n")
    return
end

for record in result:recordlist() do
    io.write("--------------------------------\n")
    for k, v in pairs(record) do
        io.write("[", k, "] -> ", fieldnamelist[k], ": ", v, "\n")
    end
end
