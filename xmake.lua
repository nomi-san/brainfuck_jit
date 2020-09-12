-- Asmjit as shared library
target("asmjit")
    set_kind("shared")
    add_defines("asmjit_EXPORTS")
    add_files("asmjit/src/asmjit/core/*.cpp")
    add_files("asmjit/src/asmjit/x86/*.cpp")

target("brainfuck_jit")
    add_deps("asmjit")
    set_kind("binary")
    add_links("asmjit")
    add_includedirs("asmjit/src/")
    add_files("brainfuck_jit.cpp")

-- Asmjit as static library
--[[
target("asmjit")
    set_kind("static")
    add_defines("asmjit_EXPORTS")
    add_files("asmjit/src/asmjit/core/*.cpp")
    add_files("asmjit/src/asmjit/x86/*.cpp")

target("brainfuck_jit")
    add_deps("asmjit")
    set_kind("binary")
    add_links("asmjit")
    add_includedirs("asmjit/src/")
    add_defines("ASMJIT_STATIC")
    add_files("brainfuck_jit.cpp")
--]]