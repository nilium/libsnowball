
solution "snowball"
configurations { "Debug-Static", "Release-Static", "Debug-Shared", "Release-Shared" }

newoption {
  trigger       = "c++11",
  description   = "Enables compilation with c++11 -- uses libc++ instead\n"..
"                   of libstdc++ as the stdlib"
}

project "snowball"
language "C++"

  files { "src/*.cc", "src/*.hh", "include/*.h" }

  includedirs { "include" }
  defines { "SZ_BUILDING" }

  flags { "ExtraWarnings" }

  configuration "c++11"
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
