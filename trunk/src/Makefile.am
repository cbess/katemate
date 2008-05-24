INCLUDES    = $(all_includes)
METASOURCES = AUTO

KDE_ICON = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = libkatemateplugin.la

libkatemateplugin_la_SOURCES = KateConfigPageWidget.ui KateMateConfigDialog.ui \
	SnippetConfigWidget.ui kateconfigpage.cpp katemateconfigdia.cpp katematexml.cpp \
	plugin_katemate.cpp snippetconfigpage.cpp snippetlistviewitem.cpp snippetprocessor.cpp
libkatemateplugin_la_LIBADD = -lkateinterfaces
libkatemateplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

pluginsdir = $(kde_datadir)/kate/plugins/katemate
plugins_DATA = plugin_katemate.rc

kde_services_DATA = katemate.desktop

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/katemate.pot

noinst_HEADERS = kateconfigpage.h katematehelpers.h katematexml.h snippet.h \
	snippetconfigpage.h snippetprocessor.h
_SOURCES = katemateconfigdia.h
_SOURCES = katemateconfigdia.h snippetlistviewitem.h
_SOURCES = katemateconfigdia.h snippetlistviewitem.h
