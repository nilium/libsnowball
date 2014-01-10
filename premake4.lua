solution "snowball"
configurations { "Debug-Static", "Release-Static", "Debug-Shared", "Release-Shared" }

project "snowball"
language "C++"

  files { "src/*.cc", "src/*.hh", "include/*.h" }

  includedirs { "include" }
  buildoptions { "-std=c++11", "-stdlib=libc++" }
  defines { "SZ_BUILDING" }

  flags { "ExtraWarnings" }

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
