cmake_minimum_required(VERSION 3.22)
project(AudioToMidi VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE is downloaded automatically on the first configure (needs internet access once).
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

set(ATM_ICON_ARGS "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo_icon.png")
    set(ATM_ICON_ARGS ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo_icon.png")
endif()

juce_add_plugin(AudioToMidi
    COMPANY_NAME "COTERIA"
    PRODUCT_NAME "ATM"
    FORMATS VST3 Standalone
    VST3_CATEGORIES Fx
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Cotr
    PLUGIN_CODE A2Mi
    ${ATM_ICON_ARGS}                    # COTERIA logo as the icon of the standalone app (if uploaded)
    COPY_PLUGIN_AFTER_BUILD FALSE)      # set TRUE to auto-install into the system VST3 folder (may need admin rights)

# the ATM picture; also accepted if it was uploaded to the repository root by mistake
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/atm_full.png")
    set(ATM_PICTURE assets/atm_full.png)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/atm_full.png")
    set(ATM_PICTURE atm_full.png)
else()
    message(FATAL_ERROR "atm_full.png not found: it must be at assets/atm_full.png in the repository")
endif()
set(ATM_BINARY ${ATM_PICTURE})
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/sfx_money_counter.wav")
    list(APPEND ATM_BINARY assets/sfx_money_counter.wav)    # the "ATM SFX" sound
    set(ATM_HAS_SFX 1)
else()
    set(ATM_HAS_SFX 0)
endif()
juce_add_binary_data(AtmAssets SOURCES ${ATM_BINARY})
set_target_properties(AtmAssets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_sources(AudioToMidi PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
    PitchToMidi.h
    Lang.h
    BasicPitchNet.h
    BasicPitchModel.h)

# address of the small version file the plugin reads to tell the user about updates (fixed address, the same for every build; empty = no update check)
set(ATM_UPDATE_URL "https://raw.githubusercontent.com/COT3RIA/ATM/main/latest.json" CACHE STRING "URL of latest.json")
if(ATM_UPDATE_URL)
    target_compile_definitions(AudioToMidi PRIVATE ATM_UPDATE_URL="${ATM_UPDATE_URL}")
endif()

target_compile_definitions(AudioToMidi PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_MP3AUDIOFORMAT=1      # MP3 decoding
    JUCE_USE_FLAC=1
    ATM_HAS_SFX=${ATM_HAS_SFX})               # FLAC decoding

target_link_libraries(AudioToMidi
    PRIVATE
        AtmAssets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
