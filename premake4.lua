solution "snowball"
configurations { "Debug", "Release" }

project "snowball"
kind "StaticLib"
language "C++"

files { "src/*.cc", "src/*.hh", "include/*.h" }

includedirs { "include" }
buildoptions { "-std=c++11", "-stdlib=libc++" }

flags { "ExtraWarnings" }

configuration "Debug"
  defines { "DEBUG" }
  flags { "Symbols" }

configuration "Release"
  defines { "NDEBUG" }
  flags { "Optimize" }