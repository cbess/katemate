
include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


set(katemateplugin_PART_SRCS
    kateconfigpage.cpp
    katemateconfigdia.cpp
    katematexml.cpp
    plugin_katemate.cpp
    snippetconfigpage.cpp
    snippetlistviewitem.cpp
    snippetprocessor.cpp)

kde4_add_ui_files(katemateplugin_PART_SRCS KateConfigPageWidget.ui KateMateConfigDialog.ui SnippetConfigWidget.ui)

kde4_add_plugin(katemateplugin WITH_PREFIX ${katemateplugin_PART_SRCS})

target_link_libraries(katemateplugin ${KDE4_KDECORE_LIBS} kateinterfaces)

install(TARGETS katemateplugin DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES  plugin_katemate.rc DESTINATION /share/apps/kate/plugins/katemate)
install(FILES  katemate.desktop DESTINATION /share/services)

kde4_install_icons(${ICON_INSTALL_DIR})

