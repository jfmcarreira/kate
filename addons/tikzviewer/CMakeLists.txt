project(katetikzviewerplugin)
add_definitions(-DTRANSLATION_DOMAIN=\"katetikzviewerplugin\")

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(katetikzviewerplugin_PART_SRCS
    katetikzviewerplugin.h
    katetikzviewerplugin.cpp
    katetikzviewerpluginview.h
    katetikzviewerpluginview.cpp
    katetikzviewerconfigpage.h
    katetikzviewerconfigpage.cpp
    katetikzviewer.h
    katetikzviewer.cpp
)

add_library(katetikzviewerplugin MODULE ${katetikzviewerplugin_PART_SRCS})

# we compile in the .desktop file
kcoreaddons_desktop_to_json (katetikzviewerplugin katetikzviewerplugin.desktop)

target_link_libraries(katetikzviewerplugin
    KF5::TextEditor
    KF5::Parts
    KF5::IconThemes
    KF5::I18n
    KF5::Service
)

install(TARGETS katetikzviewerplugin  DESTINATION ${PLUGIN_INSTALL_DIR}/ktexteditor )
