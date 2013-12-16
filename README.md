--------------------------------------------------------------------------------
2009-2013 by Jupiter Jazz Limited.

This software, excluded third party dependencies, is released in public domain,
see unlicense.txt file for more detail.

IMPORTATNT:
NUKE is a trademark of The Foundry Visionmongers Ltd.
Qt is a trademark of Digia Plc and/or its subsidiary(-ies).

--------------------------------------------------------------------------------


Introduction
============
This project is a Custom Qt knob plugin for Nuke, provides ability of showing
"Unix" style paths in a hierarchy structure. It was intended to be used in our
close source product but now released as "yet another custom Qt knob example".

This is NOT a replacement of SceneView_Knob of Nuke, use it only if suitable for
your project.

Motivation
----------
There were two main reasons for this project:
- We need a pure C-POD type interface for Nuke plugins, so that we don't have to
  worry about STL ABI compatibility issue with Nuke NDK, especially on Windows;
- We would want to get the selection state of a specified item from Nuke's
  SceneView_Knob but it was not possible in Nuke 6.x & 7.x.

Improvement
-----------
This is a feature completed project but still could be improved:
- The index information (QHash and QVector) in HierarchyViewWidget should be
  moved to a more suitable place, e.g. inherit from QModelIndex and store index
  information there;
- Extend the interface to allow retrieving specified item name/path;
- A better approach to store the selection state. Currently this information is
  stored in a std::string, this is to simplify serialization / deserialization
  when saving / loading Nuke scene.


Directory Structure
===================

|-- README.md
|-- unlicense.txt                     -- License file
+-- libHierarchyViewKnob/             -- Core library
|   |-- HierarchyViewKnob.h           -- Header file of knob (public interface)
|   |-- HierarchyViewKnob.cpp         -- Implementation of knob
|   |-- HierarchyViewWidget.moc.h     -- Qt meta-object header file
+-- HierarchyViewKnobExample/         -- Example Nuke plugin for demonstration
    |-- HierarchyViewKnobExample.cpp  -- Example source code



Compilation Guideline
=====================

libHierarchyViewKnob
--------------------
Dependency:
Nuke 6.3:
    Qt 4.6.2 with GCC 4.1 / Visual Studio 2005 SP1
Nuke 7.0:
    Qt 4.7.2 with GCC 4.1 / Visual Studio 2010


Example GCC command line on Linux:
1. Generate source file from meta-object file:
   $ moc -I<Qt_header_and_HierarchyViewKnob_header>        \
         -DQT_GUI_LIB -DQT_CORE_LIB -DNDEBUG -DLINUX       \
         -o moc_HierarchyViewWidget.cxx                    \
         HierarchyViewWidget.moc.h

2. Compile HierarchyViewKnob.cpp:
   $ gcc -DLINUX -DQT_CORE_LIB -DQT_GUI_LIB -DQT_NO_DEBUG  \
         -O3 -DNDEBUG -fPIC                                \
         -DATOM_DLL_EXPORT                                 \
         -I<Qt_header_and_HierarchyViewKnob_header>        \
         -I/usr/local/Nuke7.0v9/include                    \
         -Wall -O3                                         \
         -o HierarchyViewKnob.cpp.o                        \
         -c HierarchyViewKnob.cpp

3. Compile generated moc_HierarchyViewWidget.cxx:
   $ gcc -DLINUX -DQT_CORE_LIB -DQT_GUI_LIB -DQT_NO_DEBUG  \
         -O3 -DNDEBUG -fPIC                                \
         -DATOM_DLL_EXPORT                                 \
         -I<Qt_header_and_HierarchyViewKnob_header>        \
         -I/usr/local/Nuke7.0v9/include                    \
         -Wall -O3                                         \
         -o moc_HierarchyViewWidget.cxx.o                  \
         -c moc_HierarchyViewWidget.cxx

4. Linking shared library libHierarchyViewKnob.so:
   $ gcc -fPIC -O3 -DNDEBUG -shared                        \
         -Wl,-soname,libHierarchyViewKnob.so               \
         -L/usr/local/Nuke7.0v9 lDDImage                   \
         -L<Qt_library_dir> -lQtCore -lQtGui               \
         -o libHierarchyViewKnob.so                        \
         HierarchyViewKnob.cpp.o                           \
         moc_HierarchyViewWidget.cxx.o


HierarchyViewKnob Example Plugin
--------------------------------
Dependency:
  libHierarchyViewKnob.so

Example GCC command line on Linux:
$ gcc -DATOM_DLL_IMPORT -DLINUX -DNDEBUG -fPIC -Wall       \
      -O3 -shared                                          \
      -L/usr/local/Nuke7.0v9 -lDDImage                     \
      -L<path_of_libHierarchyViewKnob> -lHierarchyViewKnob \
      -o HierarchyViewKnobExample.so                       \
      HierarchyViewKnobExample.cpp

No Qt dependency for HierarchyViewKnobExample here, since the detail
implementation is hidden inside libHierarchyViewKnob.so.


