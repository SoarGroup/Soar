-- Name: testfile.lua
--
--
--==================================================
--==================================================

dofile("LuaFiles/common.lua")
dofile("LuaFiles/test1.lua")
dofile("LuaFiles/test2.lua")
dofile("LuaFiles/test3.lua")
dofile("LuaFiles/test4.lua")
dofile("LuaFiles/test4a.lua")
dofile("LuaFiles/test5.lua")
dofile("LuaFiles/test6.lua")
dofile("LuaFiles/test7.lua")
dofile("LuaFiles/test8.lua")
dofile("LuaFiles/addwme.lua")
dofile("LuaFiles/misc_commands.lua")

----------------------------------------------------
--
----------------------------------------------------
function main()
   setCompression(nil)
   --test1()
   test2()
   test3()
   test4()
   test4a()
   test5()
   test6()
   test7()
   test8()
   AddWmeTest()
   TestMiscCommands()
end

----------------------------------------------------
--
----------------------------------------------------
main()

