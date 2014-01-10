
solution "snowball"
configurations {
  "Debug-Static",
  "Release-Static",
  "Debug-Shared",
  "Release-Shared"
}

newoption {
  trigger       = "c++98",
  description   = "Disables compilation with c++11 -- uses libstdc++\n"..
"                   in place of libc++ and compiles as C++98."
}

project "snowball"
language "C++"

  files { "src/*.cc", "src/*.hh", "include/*.h" }

  includedirs { "include" }
  defines { "SZ_BUILDING" }

  flags { "ExtraWarnings" }

  configuration "not c++98"
    buildoptions { "-std=c++11", "-stdlib=libc++" }

  configuration "Debug-*"
    defines { "DEBUG" }
    flags { "Symbols" }

  configuration "Release-*"
    defines { "NDEBUG" }
    flags { "Optimize" }

  configuration "*-Static"
    kind "StaticLib"

  configuration "*-Shared"
    kind "SharedLib"
