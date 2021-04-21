cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Spell)

set(Spell_Source_Files
        SpellFactory.cpp
)

add_library(Spell ${Spell_Source_Files})
