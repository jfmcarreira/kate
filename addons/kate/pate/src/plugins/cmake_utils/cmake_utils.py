# -*- coding: utf-8 -*-
'''CMake helper plugin'''

#
# CMake helper plugin
#
# Copyright 2013 by Alex Turbov <i.zaufi@gmail.com>
#
# This software is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this software.  If not, see <http://www.gnu.org/licenses/>.


import imp
import functools
import glob
import os
import re
import subprocess
import sys
import types

from PyQt4 import uic
from PyQt4.QtCore import QEvent, QObject, QUrl, Qt, pyqtSlot
from PyQt4.QtGui import (
    QCheckBox
  , QSizePolicy
  , QSpacerItem
  , QSplitter
  , QTabWidget
  , QTextBrowser
  , QTreeWidget
  , QTreeWidgetItem
  , QVBoxLayout
  , QWidget
  )

from PyKDE4.kdecore import i18nc, KUrl
from PyKDE4.kio import KFile, KUrlRequester
from PyKDE4.ktexteditor import KTextEditor

import kate

from libkatepate import ui, common
from libkatepate.autocomplete import AbstractCodeCompletionModel

from cmake_utils_settings import *
import cmake_help_parser


cmakeToolView = None

# ----------------------------------------------------------
# CMake utils: completion stuff
# ----------------------------------------------------------

class CMakeCompletionModel(AbstractCodeCompletionModel):
    '''Completion model for CMake files'''
    # TODO Unit tests

    TITLE_AUTOCOMPLETION = i18nc('@label:listbox', 'CMake Auto Completion')
    MAX_DESCRIPTION = 100

    _cc_registrar_fn_name = 'register_command_completer'

    def __init__(self, app):
        super(CMakeCompletionModel, self).__init__(app)
        # Create an empty dict of command completers
        self.__command_completers = {}
        # Try to load all available particular command completers
        self._loadCompleters()

    def reset(self):
        '''Reset the model'''
        self.resultList = []

    def completionInvoked(self, view, word, invocationType):
        '''Completion has been inviked for given view and word'''
        # First of all check the document's MIME-type
        document = view.document()
        mimetype = document.mimeType()
        if mimetype != 'text/x-cmake':
            return

        print('CMakeCC [{}]: current word: "{}"'.format(mimetype, word))

        self.reset()                                        # Drop previously collected completions

        cursor = view.cursorPosition()
        # Try to detect completion context
        command, in_a_string, in_a_var, fn_params_range = self.find_current_context(document, cursor)
        print('CMakeCC: command="{}", in_a_string={}, in_a_var={}'.format(command, in_a_string, in_a_var))
        if fn_params_range.isValid():
            print('CMakeCC: params="{}"'.format(document.text(fn_params_range)))

        if in_a_var:
            # Try to complete a variable name
            self.TITLE_AUTOCOMPLETION = i18nc('@label:listbox', 'CMake Variables Completion')
            for var in cmake_help_parser.get_cmake_vars():
                self.resultList.append(
                    self.createItemAutoComplete(
                        text=var[0]
                      , category='constant'
                      , description=var[1]
                      )
                  )
            return
        if in_a_string:
            # If we a not in a variable expansion, but inside a string
            # there is nothing to complete!
            return
        # Try to complete a command
        if not command:
            # Try to complete a variable name
            self.TITLE_AUTOCOMPLETION = i18nc('@label:listbox', 'CMake Commands Completion')
            for cmd in cmake_help_parser.get_cmake_commands():
                self.resultList.append(
                    self.createItemAutoComplete(
                        text=cmd[0]
                      , category='function'
                      , description=cmd[1]
                      )
                  )
            return
        # Try to complete a particular command
        # 0) assemble parameter list preceded to the current completion position
        # TODO Unit tests
        text = document.text(fn_params_range).strip().split()
        found_string = False
        comp_list = []
        for tok in text:
            if tok.startswith('"'):
                comp_list.append(tok)
                found_string = True
            elif tok.endswith('"'):
                comp_list[-1] += ' ' + tok
                found_string = False
            elif found_string:
                comp_list[-1] += ' ' + tok
            else:
                comp_list.append(tok)
        # 1) call command completer
        self.try_complete_command(command, document, cursor, word, comp_list)


    def executeCompletionItem(self, document, word, row):
        # TODO Why this method is not called???
        print('CMakeCC: executeCompletionItem: ' + repr(word)+', row='+str(row))


    def find_current_context(self, document, cursor):
        '''Determinate current context under cursor'''
        # Parse whole document starting from a very first line!
        in_a_string = False
        in_a_command = False
        skip_next = False
        nested_var_level = 0
        command = None
        fn_params_start = None
        for current_line in range(0, cursor.line() + 1):
            line_str = document.line(current_line)
            prev = None
            should_count_pos = (current_line == cursor.line())
            for pos, c in enumerate(line_str):
                if should_count_pos and pos == cursor.column():
                    break
                if c == '#' and not in_a_string:
                    # TODO Syntax error if we r in a var expansion
                    break                                   # Ignore everything till the end of line
                if skip_next:                               # Should we skip current char?
                    skip_next = False                       # Yep!
                elif c == '\\':                             # Found a backslash:
                    skip_next = True                        #  skip next char!
                elif c == '"':                              # Found a quote char
                    in_a_string = not in_a_string           # Switch 'in a string' state
                    # TODO Syntax error if we r in a var expansion
                elif c == '{' and prev == '$':              # Looks like a variable expansion?
                    nested_var_level += 1                   # Yep, incrase var level
                elif c == '}':                              # End of a variable expansion
                    nested_var_level -= 1
                elif c == '(' and not in_a_string:          # Command params started
                    command = line_str[0:pos].strip()
                    # TODO Syntax error if we r in a var expansion
                    in_a_command = True
                    fn_params_start = KTextEditor.Cursor(current_line, pos + 1)
                elif c == ')' and not in_a_string:
                    # TODO Syntax error if we r in a var expansion
                    in_a_command = False
                    command = None
                    fn_params_start = None

                # TODO Handle generator expressions

                # Remember current char in a `prev' for next iteration
                prev = c
        if fn_params_start is not None:
            fn_params_range = KTextEditor.Range(fn_params_start, cursor)
        else:
            fn_params_range = KTextEditor.Range(-1, -1, -1, -1)
        return (command, in_a_string, nested_var_level != 0, fn_params_range)


    def _loadCompleters(self):
        # Load available command completers
        for directory in kate.applicationDirectories('cmake_utils/command_completers'):
            print('CMakeCC: directory={}'.format(directory))
            sys.path.append(directory)
            for completer in glob.glob(os.path.join(directory, '*_cc.py')):
                print('CMakeCC: completer={}'.format(completer))
                cc_name = os.path.basename(completer).split('.')[0]
                module = imp.load_source(cc_name, completer)
                if hasattr(module, self._cc_registrar_fn_name):
                    r = getattr(module, self._cc_registrar_fn_name)
                    if isinstance(r, types.FunctionType):
                        r(self.__command_completers)


    def try_complete_command(self, command, document, cursor, word, comp_list):
        '''Try to complete a command'''
        if command in self.__command_completers:
            if isinstance(self.__command_completers[command], types.FunctionType):
                # If a function registered as a completer, just call it...
                completions = self.__command_completers[command](document, cursor, word, comp_list)
            else:
                # Everything else, that is not a function, just pass to the generic completer
                completions = self._try_syntactic_completer(
                    self.__command_completers[command]
                  , document
                  , cursor
                  , word
                  , comp_list
                  )
        else:
            ui.popup(
                i18nc('@title:window', 'Attention')
              , i18nc('@info:tooltip', 'Sorry, no completion for <command>{}()</command>'.format(command))
              , 'dialog-information'
              )

            completions = []

        # Result of a completion function must be a list type
        if completions and isinstance(completions, list):
            self.TITLE_AUTOCOMPLETION = i18nc(
                '@label:listbox'
              , 'CMake <command>{}()</command> Completion'.format(command)
              )
            for c in completions:
                # If completion item is a tuple, we expect to have 2 items in it:
                # 0 is a 'text' and 1 is a 'description'
                if isinstance(c, tuple) and len(c) == 2:
                    self.resultList.append(
                        self.createItemAutoComplete(
                            text=c[0]
                        , description=c[1]
                        )
                    )
                else:
                    self.resultList.append(self.createItemAutoComplete(text=c))


    def _try_syntactic_completer(self, syntax, document, cursor, word, comp_list):
        print('CMakeCC: generic completer: syntax='+str(syntax))
        print('CMakeCC: generic completer: comp_list='+str(comp_list))
        result = []
        if isinstance(syntax, list):
            for sid, s in enumerate(syntax):
                (items, stop) = s.complete(document, cursor, word, comp_list, sid)
                if stop:
                    return items
                result += items
        else:
            (items, stop) = syntax.complete(document, cursor, word, comp_list)
            result = items
        print('CMakeCC: generic completer result={}'.format(result))
        # TODO sort | uniq
        return result


def _reset(*args, **kwargs):
    cmake_completation_model.reset()


# ----------------------------------------------------------
# CMake utils: toolview
# ----------------------------------------------------------

class CMakeToolView(QObject):
    '''CMake tool view class

        TODO Remember last dir/position/is-advanced?
        TODO Make the cache view editable and run `cmake` to reconfigure
    '''
    cmakeCache = []

    def __init__(self, parent):
        super(CMakeToolView, self).__init__(parent)
        self.toolView = kate.mainInterfaceWindow().createToolView(
            'cmake_utils'
          , kate.Kate.MainWindow.Bottom
          , kate.gui.loadIcon('cmake')
          , i18nc('@title:tab', 'CMake')
          )
        self.toolView.installEventFilter(self)
        # By default, the toolview has box layout, which is not easy to delete.
        # For now, just add an extra widget.
        tabs = QTabWidget(self.toolView)
        # Make a page to view cmake cache
        cacheViewPage = QWidget(tabs)
        self.buildDir = KUrlRequester(cacheViewPage)
        self.buildDir.setText(kate.configuration[PROJECT_DIR])
        # TODO It seems not only KTextEditor's SIP files are damn out of date...
        # KUrlRequester actually *HAS* setPlaceholderText() method... but damn SIP
        # files for KIO are damn out of date either! A NEW BUG NEEDS TO BE ADDED!
        # (but I have fraking doubts that it will be closed next few damn years)
        #
        #self.buildDir.setPlaceholderText(i18nc('@info', 'Project build directory'))
        self.buildDir.lineEdit().setPlaceholderText(i18nc('@info/plain', 'Project build directory'))
        self.buildDir.setMode(KFile.Mode(KFile.Directory | KFile.ExistingOnly | KFile.LocalOnly))
        self.cacheItems = QTreeWidget(cacheViewPage)
        self.cacheItems.setHeaderLabels((
            i18nc('@title:column', 'Name')
          , i18nc('@title:column', 'Type')
          , i18nc('@title:column', 'Value')
          ))
        self.cacheItems.setSortingEnabled(True)
        self.cacheItems.sortItems(0, Qt.AscendingOrder)
        layout_p1 = QVBoxLayout(cacheViewPage)
        layout_p1.addWidget(self.buildDir)
        layout_p1.addWidget(self.cacheItems)
        tabs.addTab(cacheViewPage, i18nc('@title:tab', 'CMake Cache Viewer'))
        # Make a page w/ cmake help
        splitter = QSplitter(Qt.Horizontal, tabs)
        self.helpTargets = QTreeWidget(splitter)
        self.helpTargets.setToolTip(
            i18nc('@info:tooltip', 'Double-click to insert the current item to a document')
          )
        self.helpTargets.headerItem().setHidden(True)
        self.updateHelpIndex()                              # Prepare Help view
        self.helpPage = QTextBrowser(splitter)
        self.helpPage.setReadOnly(True)
        self.helpPage.setOpenExternalLinks(False)
        self.helpPage.setOpenLinks(False)
        splitter.addWidget(self.helpTargets)
        splitter.addWidget(self.helpPage)
        tabs.addTab(splitter, i18nc('@title:tab', 'CMake Help'))
        # Make a page w/ some instant settings
        cfgPage = QWidget(tabs)
        self.mode = QCheckBox(i18nc('@option:check', 'Show cache items marked as advanced'), cfgPage)
        self.mode.setChecked(kate.configuration[TOOLVIEW_ADVANCED_MODE])
        self.mode.setToolTip(
            i18nc('@info:tooltip', 'Same as <emphasis>advanced mode</emphasis> in <command>ccmake</command>')
          )
        self.htmlize = QCheckBox(i18nc('@option:check', 'Try to beautify the help output'), cfgPage)
        self.htmlize.setChecked(kate.configuration[TOOLVIEW_BEAUTIFY])
        self.htmlize.setToolTip(
            i18nc(
                '@info:tooltip'
              , 'Try to turn plain ASCII text into HTML using simple structured text rules. ' \
                'It works quite well for builtins, but sometimes may distorts results for others ' \
                '(custom modules)!'
              )
          )
        layout_p3 = QVBoxLayout(cfgPage)
        layout_p3.addSpacerItem(QSpacerItem(10, 10, QSizePolicy.Minimum, QSizePolicy.Minimum))
        layout_p3.addWidget(self.mode)
        layout_p3.addWidget(self.htmlize)
        layout_p3.addSpacerItem(QSpacerItem(10, 10, QSizePolicy.Minimum, QSizePolicy.Expanding))
        tabs.addTab(cfgPage, i18nc('@title:tab', 'Toolview Settings'))
        # TODO Store check-boxes state to configuration

        # Connect signals
        self.cacheItems.itemActivated.connect(self.insertIntoCurrentDocument)
        self.buildDir.returnPressed.connect(self.updateCacheView)
        self.buildDir.urlSelected.connect(self.updateCacheView)
        self.mode.toggled.connect(self.updateCacheView)
        self.mode.toggled.connect(self.saveSettings)
        self.htmlize.toggled.connect(self.updateHelpText)
        self.htmlize.toggled.connect(self.saveSettings)
        self.helpTargets.itemActivated.connect(self.updateHelpText)
        self.helpTargets.itemDoubleClicked.connect(self.insertHelpItemIntoCurrentDocument)
        self.helpPage.anchorClicked.connect(self.openDocument)

        # Refresh the cache view
        self._updateCacheView(self.buildDir.text())


    def __del__(self):
        """Plugins that use a toolview need to delete it for reloading to work."""
        if self.toolView:
            self.toolView.deleteLater()
            self.toolView = None


    def eventFilter(self, obj, event):
        """Hide the Palette tool view on ESCAPE key"""
        if event.type() == QEvent.KeyPress and event.key() == Qt.Key_Escape:
            kate.mainInterfaceWindow().hideToolView(self.toolView)
            return True
        return self.toolView.eventFilter(obj, event)


    @pyqtSlot()
    def saveSettings(self):
        kate.configuration[TOOLVIEW_ADVANCED_MODE] = self.mode.isChecked()
        kate.configuration[TOOLVIEW_BEAUTIFY] = self.htmlize.isChecked()


    @pyqtSlot()
    def updateCacheView(self):
        self._updateCacheView(self.buildDir.text())


    def _updateCacheView(self, build_dir):
        # Do nothing if build dir is not configured
        if not build_dir:
            return

        self.cacheItems.clear()                             # Remove previously collected cache
        is_advanced = self.mode.isChecked()

        try:
            items = cmake_help_parser.get_cache_content(build_dir, is_advanced)
            print('CMakeCC: update cache view: result={}'.format(items))
        except ValueError as error:
            ui.popup(
                i18nc('@title:window', 'Error')
              , i18nc('@info:tooltip', 'Unable to get CMake cache content:<nl/><message>{}</message>'.format(error))
              , 'dialog-error'
              )
            return

        # Add items to a list
        for key, value in items.items():
            item = QTreeWidgetItem(self.cacheItems, [key, value[1], value[0]])
            item.setToolTip(0, value[2])

        self.cacheItems.resizeColumnToContents(0)
        self.cacheItems.resizeColumnToContents(1)
        self.cacheItems.resizeColumnToContents(2)


    def updateHelpIndex(self):
        #
        commands = QTreeWidgetItem(
            self.helpTargets
          , [i18nc('@item::inlistbox/plain', 'Commands')]
          , cmake_help_parser.help_category.COMMAND
          )
        for cmd in cmake_help_parser.get_cmake_commands_list():
            c = QTreeWidgetItem(commands, [cmd])
        #
        modules = QTreeWidgetItem(
            self.helpTargets
          , [i18nc('@item::inlistbox/plain', 'Modules')]
          , cmake_help_parser.help_category.MODULE
          )
        for mod in cmake_help_parser.get_cmake_modules_list():
            m = QTreeWidgetItem(modules, [mod])
        #
        policies = QTreeWidgetItem(
            self.helpTargets
          , [i18nc('@item::inlistbox/plain', 'Policies')]
          , cmake_help_parser.help_category.POLICY
          )
        for pol in cmake_help_parser.get_cmake_policies_list():
            p = QTreeWidgetItem(policies, [pol])
        #
        properties = QTreeWidgetItem(
            self.helpTargets
          , [i18nc('@item::inlistbox/plain', 'Properties')]
          , cmake_help_parser.help_category.PROPERTY
          )
        for prop in cmake_help_parser.get_cmake_properties_list():
            p = QTreeWidgetItem(properties, [prop])
        #
        variables = QTreeWidgetItem(
            self.helpTargets
          , [i18nc('@item::inlistbox/plain', 'Variables')]
          , cmake_help_parser.help_category.VARIABLE
          )
        for var in cmake_help_parser.get_cmake_vars_list():
            v = QTreeWidgetItem(variables, [var])

        self.helpTargets.resizeColumnToContents(0)


    @pyqtSlot()
    def updateHelpText(self):
        tgt = self.helpTargets.currentItem()
        if tgt is None:
            return
        parent = tgt.parent()
        if parent is None:
            return

        category = parent.type()
        text = cmake_help_parser.get_help_on(category, tgt.text(0))

        if not self.htmlize.isChecked():
            self.helpPage.setText(text[text.index('\n') + 1:])
            return

        # TODO How *else* we can beautify the text?
        lines = text.splitlines()[1:]
        file_link_re = re.compile('Defined in: (.*)')
        file_link_sb = 'Defined in: <a href="file://\\1">\\1</a>'
        pre = False
        para = True
        for i, line in enumerate(lines):
            # Remove '&', '<' and '>' from text
            # TODO Use some HTML encoder instead of this...
            line = line.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
            #
            if line.lstrip().startswith('Defined in: '):
                line = file_link_re.sub(file_link_sb, line)
            #
            if i == 0:
                line = '<h1>{}</h1>'.format(line)
            elif line.startswith(' ' * cmake_help_parser.CMAKE_HELP_VARBATIM_TEXT_PADDING_SIZE):
                if not pre:
                    line = '<pre>' + line
                    pre = True
            elif len(line.strip()) == 0:
                if pre:
                    line = line + '</pre>'
                    pre = False
                elif para:
                    line = line + '</p>'
                    para = False
                else:
                    line = '<p>' + line
                    para = True
            lines[i] = line
        self.helpPage.setText('\n'.join(lines))


    @pyqtSlot(QTreeWidgetItem, int)
    def insertIntoCurrentDocument(self, item, column):
        if item is not None and column == 0:
            view = kate.activeView()
            document = kate.activeDocument()
            document.startEditing()
            document.insertText(view.cursorPosition(), item.text(0))
            document.endEditing()

    @pyqtSlot(QTreeWidgetItem, int)
    def insertHelpItemIntoCurrentDocument(self,item, column):
        if item is not None and item.parent() is not None and column == 0:
            view = kate.activeView()
            document = kate.activeDocument()
            document.startEditing()
            document.insertText(view.cursorPosition(), item.text(0))
            document.endEditing()


    @pyqtSlot(QUrl)
    def openDocument(self, url):
        local_file = url.toLocalFile()
        print('CMakeCC: going to open the document: {}'.format(local_file))
        if os.access(local_file, os.R_OK):
            document = kate.documentManager.openUrl(KUrl(url))
            document.setReadWrite(os.access(local_file, os.W_OK))
            kate.application.activeMainWindow().activateView(document)
        else:
            ui.popup(
                i18nc('@title:window', 'Error')
              , i18nc('@info:tooltip', 'Unable to open the document: <filename>{}</filename>'.format(local_file))
              , 'dialog-error'
              )


# ----------------------------------------------------------
# CMake utils: configuration stuff
# ----------------------------------------------------------

class CMakeConfigWidget(QWidget):
    '''Configuration widget for this plugin.'''

    cmakeBinary = None
    projectBuildDir = None

    def __init__(self, parent=None, name=None):
        super(CMakeConfigWidget, self).__init__(parent)

        # Set up the user interface from Designer.
        uic.loadUi(os.path.join(os.path.dirname(__file__), CMAKE_UTILS_SETTINGS_UI), self)

        self.reset();

    def apply(self):
        kate.configuration[CMAKE_BINARY] = self.cmakeBinary.text()
        try:
            cmake_help_parser.validate_cmake_executable(kate.configuration[CMAKE_BINARY])
        except ValueError as error:
            ui.popup(
                i18nc('@title:window', 'Error')
              , i18nc('@info:tooltip', 'CMake executable test run failed:<nl/><message>{}</message>'.format(error))
              , 'dialog-error'
              )
        # TODO Store the following for a current session!
        kate.configuration[PROJECT_DIR] = self.projectBuildDir.text()
        kate.configuration.save()

    def reset(self):
        self.defaults()
        if CMAKE_BINARY in kate.configuration:
            self.cmakeBinary.setText(kate.configuration[CMAKE_BINARY])

    def defaults(self):
        # TODO Dectect it!
        self.cmakeBinary.setText(CMAKE_BINARY_DEFAULT)
        self.projectBuildDir.setText('')


class CMakeConfigPage(kate.Kate.PluginConfigPage, QWidget):
    '''Kate configuration page for this plugin.'''
    def __init__(self, parent=None, name=None):
        super(CMakeConfigPage, self).__init__(parent, name)
        self.widget = CMakeConfigWidget(parent)
        lo = parent.layout()
        lo.addWidget(self.widget)

    def apply(self):
        self.widget.apply()

    def reset(self):
        self.widget.reset()

    def defaults(self):
        self.widget.defaults()
        self.changed.emit()


@kate.configPage(
    i18nc('@action:inmenu', 'CMake Helper Plugin')
  , i18nc('@title:group', 'CMake Helper Settings')
  , icon='cmake'
  )
def cmakeConfigPage(parent=None, name=None):
    return CMakeConfigPage(parent, name)

@kate.viewCreated
def createSignalAutocompleteCMake(view=None, *args, **kwargs):
    print('CMakeCC: Register completion model')
    try:
        view = view or kate.activeView()
        if view:
            cci = view.codeCompletionInterface()
            cci.registerCompletionModel(cmake_completation_model)
    except:
        print('CMake Helper Plugin: Unable to get an active view')


@kate.init
def init():
    # Set default value if not configured yet
    print('CMakeCC: enter init')
    if CMAKE_BINARY not in kate.configuration:
        kate.configuration[CMAKE_BINARY] = CMAKE_BINARY_DEFAULT
    if PROJECT_DIR not in kate.configuration:
        kate.configuration[PROJECT_DIR] = ''
    if TOOLVIEW_ADVANCED_MODE not in kate.configuration:
        kate.configuration[TOOLVIEW_ADVANCED_MODE] = False
    if TOOLVIEW_BEAUTIFY not in kate.configuration:
        kate.configuration[TOOLVIEW_BEAUTIFY] = True

    print('CMakeCC: init: cmakeBinary='.format(kate.configuration[CMAKE_BINARY]))

    # Initialize completion model
    createSignalAutocompleteCMake()

    # Make an instance of a cmake tool view
    global cmakeToolView
    if cmakeToolView is None:
        cmakeToolView = CMakeToolView(kate.mainWindow())


@kate.unload
def destroy():
    '''Plugins that use a toolview need to delete it for reloading to work.'''
    global cmakeToolView
    if cmakeToolView:
        cmakeToolView.__del__()
        cmakeToolView = None

cmake_completation_model = CMakeCompletionModel(kate.application)
cmake_completation_model.modelReset.connect(_reset)


# kate: indent-width 4;