# XFNoParsing
# Program: template
# Description: file selector box
#
# $Header$

global fsBox
set fsBox(activeBackground) ""
set fsBox(activeForeground) ""
set fsBox(background) ""
set fsBox(font) ""
set fsBox(foreground) ""
set fsBox(scrollActiveForeground) ""
set fsBox(scrollBackground) ""
set fsBox(scrollForeground) ""
set fsBox(scrollSide) left
set fsBox(showPixmap) 0
set fsBox(name) ""
set fsBox(path) [pwd]
set fsBox(pattern) *
set fsBox(all) 0
set fsBox(button) 0
set fsBox(extensions) 0
set fsBox(internalPath) [pwd]

proc FSBox {{fsBoxMessage "Select file:"} {fsBoxFileName ""} {fsBoxActionOk ""} {fsBoxActionCancel ""}} {# xf ignore me 5
##########
# Procedure: FSBox
# Description: show file selector box
# Arguments: fsBoxMessage - the text to display
#            fsBoxFileName - a file name that should be selected
#            fsBoxActionOk - the action that should be performed on ok
#            fsBoxActionCancel - the action that should be performed on cancel
# Returns: the filename that was selected, or nothing
# Sideeffects: none
##########
# 
# global fsBox(activeBackground) - active background color
# global fsBox(activeForeground) - active foreground color
# global fsBox(background) - background color
# global fsBox(font) - text font
# global fsBox(foreground) - foreground color
# global fsBox(extensions) - scan directory for extensions
# global fsBox(scrollActiveForeground) - scrollbar active background color
# global fsBox(scrollBackground) - scrollbar background color
# global fsBox(scrollForeground) - scrollbar foreground color
# global fsBox(scrollSide) - side where scrollbar is located

  global fsBox

  set tmpButtonOpt ""
  set tmpFrameOpt ""
  set tmpMessageOpt ""
  set tmpScaleOpt ""
  set tmpScrollOpt ""
  if {"$fsBox(activeBackground)" != ""} {
    append tmpButtonOpt "-activebackground \"$fsBox(activeBackground)\" "
  }
  if {"$fsBox(activeForeground)" != ""} {
    append tmpButtonOpt "-activeforeground \"$fsBox(activeForeground)\" "
  }
  if {"$fsBox(background)" != ""} {
    append tmpButtonOpt "-background \"$fsBox(background)\" "
    append tmpFrameOpt "-background \"$fsBox(background)\" "
    append tmpMessageOpt "-background \"$fsBox(background)\" "
  }
  if {"$fsBox(font)" != ""} {
    append tmpButtonOpt "-font \"$fsBox(font)\" "
    append tmpMessageOpt "-font \"$fsBox(font)\" "
  }
  if {"$fsBox(foreground)" != ""} {
    append tmpButtonOpt "-foreground \"$fsBox(foreground)\" "
    append tmpMessageOpt "-foreground \"$fsBox(foreground)\" "
  }
  if {"$fsBox(scrollActiveForeground)" != ""} {
    append tmpScrollOpt "-activeforeground \"$fsBox(scrollActiveForeground)\" "
  }
  if {"$fsBox(scrollBackground)" != ""} {
    append tmpScrollOpt "-background \"$fsBox(scrollBackground)\" "
  }
  if {"$fsBox(scrollForeground)" != ""} {
    append tmpScrollOpt "-foreground \"$fsBox(scrollForeground)\" "
  }

  if {[file exists [file tail $fsBoxFileName]] &&
      [IsAFile [file tail $fsBoxFileName]]} {
    set fsBox(name) [file tail $fsBoxFileName]
  } {
    set fsBox(name) ""
  }
  if {[file exists $fsBoxFileName] && [IsADir $fsBoxFileName]} {
    set fsBox(path) $fsBoxFileName
  } {
    if {"[file rootname $fsBoxFileName]" != "."} {
      set fsBox(path) [file rootname $fsBoxFileName]
    }
  }
  if {$fsBox(showPixmap)} {
    set fsBox(path) [string trimleft $fsBox(path) @]
  }
  if {"$fsBox(path)" != "" && [file exists $fsBox(path)] &&
      [IsADir $fsBox(path)]} {
    set fsBox(internalPath) $fsBox(path)
  } {
    if {"$fsBox(internalPath)" == "" ||
        ![file exists $fsBox(internalPath)]} {
      set fsBox(internalPath) [pwd]
    }
  }
  # build widget structure

  # start build of toplevel
  if {"[info commands XFDestroy]" != ""} {
    catch {XFDestroy .fsBox}
  } {
    catch {destroy .fsBox}
  }
  toplevel .fsBox \
    -borderwidth 0
  catch ".fsBox config $tmpFrameOpt"
  wm geometry .fsBox 350x300 
  wm title .fsBox {File select box}
  wm maxsize .fsBox 1000 1000
  wm minsize .fsBox 100 100
  # end build of toplevel

  label .fsBox.message1 \
    -anchor c \
    -relief raised \
    -text "$fsBoxMessage"
  catch ".fsBox.message1 config $tmpMessageOpt"

  frame .fsBox.frame1 \
    -borderwidth 0 \
    -relief raised
  catch ".fsBox.frame1 config $tmpFrameOpt"

  button .fsBox.frame1.ok \
    -text "OK" \
    -command "
      global fsBox
      set fsBox(name) \[.fsBox.file.file get\]
      if {$fsBox(showPixmap)} {
        set fsBox(path) @\[.fsBox.path.path get\]
      } {
        set fsBox(path) \[.fsBox.path.path get\]
      }
      set fsBox(internalPath) \[.fsBox.path.path get\]
      $fsBoxActionOk
      if {\"\[info commands XFDestroy\]\" != \"\"} {
        catch {XFDestroy .fsBox}
      } {
        catch {destroy .fsBox}
      }"
  catch ".fsBox.frame1.ok config $tmpButtonOpt"

  button .fsBox.frame1.rescan \
    -text "Rescan" \
    -command {
      global fsBox
      FSBoxFSShow [.fsBox.path.path get] \
        [.fsBox.pattern.pattern get] $fsBox(all)}
  catch ".fsBox.frame1.rescan config $tmpButtonOpt"

  button .fsBox.frame1.cancel \
    -text "Cancel" \
    -command "
      global fsBox
      set fsBox(name) {}
      set fsBox(path) {}
      $fsBoxActionCancel
      if {\"\[info commands XFDestroy\]\" != \"\"} {
        catch {XFDestroy .fsBox}
      } {
        catch {destroy .fsBox}
      }"
  catch ".fsBox.frame1.cancel config $tmpButtonOpt"

  if {$fsBox(showPixmap)} {
    frame .fsBox.frame2 \
      -borderwidth 0 \
      -relief raised
    catch ".fsBox.frame2 config $tmpFrameOpt"

    scrollbar .fsBox.frame2.scrollbar3 \
      -command {.fsBox.frame2.canvas2 xview} \
      -orient {horizontal} \
      -relief {raised}
    catch ".fsBox.frame2.scrollbar3 config $tmpScrollOpt"

    scrollbar .fsBox.frame2.scrollbar1 \
      -command {.fsBox.frame2.canvas2 yview} \
      -relief {raised}
    catch ".fsBox.frame2.scrollbar1 config $tmpScrollOpt"

    canvas .fsBox.frame2.canvas2 \
      -confine {true} \
      -relief {raised} \
      -scrollregion {0c 0c 20c 20c} \
      -width {100} \
      -xscrollcommand {.fsBox.frame2.scrollbar3 set} \
      -yscrollcommand {.fsBox.frame2.scrollbar1 set}
    catch ".fsBox.frame2.canvas2 config $tmpFrameOpt"

    .fsBox.frame2.canvas2 addtag currentBitmap withtag [.fsBox.frame2.canvas2 create bitmap 5 5 -anchor nw]
  }

  frame .fsBox.path \
    -borderwidth 0 \
    -relief raised
  catch ".fsBox.path config $tmpFrameOpt"

  frame .fsBox.path.paths \
    -borderwidth 2 \
    -relief raised
  catch ".fsBox.path.paths config $tmpFrameOpt"

  menubutton .fsBox.path.paths.paths \
    -borderwidth 0 \
    -menu ".fsBox.path.paths.paths.menu" \
    -relief flat \
    -text "Pathname:"
  catch ".fsBox.path.paths.paths config $tmpButtonOpt"

  menu .fsBox.path.paths.paths.menu
  catch ".fsBox.path.paths.paths.menu config $tmpButtonOpt"

  .fsBox.path.paths.paths.menu add command \
     -label "[string trimright $fsBox(internalPath) {/@}]" \
     -command "
       global fsBox
       FSBoxFSShow \[.fsBox.path.path get\] \
         \[.fsBox.pattern.pattern get\] \$fsBox(all)
       .fsBox.path.path delete 0 end
       .fsBox.path.path insert 0 [string trimright $fsBox(internalPath) {/@}]"

  entry .fsBox.path.path \
    -relief raised
  catch ".fsBox.path.path config $tmpMessageOpt"

  if {![IsADir $fsBox(internalPath)]} {
    set $fsBox(internalPath) [pwd]
  }
  .fsBox.path.path insert 0 $fsBox(internalPath)

  frame .fsBox.pattern \
    -borderwidth 0 \
    -relief raised
  catch ".fsBox.pattern config $tmpFrameOpt"

  frame .fsBox.pattern.patterns \
    -borderwidth 2 \
    -relief raised
  catch ".fsBox.pattern.patterns config $tmpFrameOpt"

  menubutton .fsBox.pattern.patterns.patterns \
    -borderwidth 0 \
    -menu ".fsBox.pattern.patterns.patterns.menu" \
    -relief flat \
    -text "Selection pattern:"
  catch ".fsBox.pattern.patterns.patterns config $tmpButtonOpt"

  menu .fsBox.pattern.patterns.patterns.menu
  catch ".fsBox.pattern.patterns.patterns.menu config $tmpButtonOpt"

  .fsBox.pattern.patterns.patterns.menu add checkbutton \
    -label "Scan extensions" \
    -variable fsBoxExtensions \
    -command {
      global fsBox
      FSBoxFSShow [.fsBox.path.path get] \
        [.fsBox.pattern.pattern get] $fsBox(all)}

  entry .fsBox.pattern.pattern \
    -relief raised
  catch ".fsBox.pattern.pattern config $tmpMessageOpt"

  .fsBox.pattern.pattern insert 0 $fsBox(pattern)
  
  frame .fsBox.files \
    -borderwidth 0 \
    -relief raised
  catch ".fsBox.files config $tmpFrameOpt"

  scrollbar .fsBox.files.vscroll \
    -relief raised \
    -command ".fsBox.files.files yview"
  catch ".fsBox.files.vscroll config $tmpScrollOpt"

  scrollbar .fsBox.files.hscroll \
    -orient horiz \
    -relief raised \
    -command ".fsBox.files.files xview"
  catch ".fsBox.files.hscroll config $tmpScrollOpt"

  listbox .fsBox.files.files \
    -exportselection false \
    -relief raised \
    -xscrollcommand ".fsBox.files.hscroll set" \
    -yscrollcommand ".fsBox.files.vscroll set"
  catch ".fsBox.files.files config $tmpMessageOpt"

  frame .fsBox.file \
    -borderwidth 0 \
    -relief raised
  catch ".fsBox.file config $tmpFrameOpt"

  label .fsBox.file.labelfile \
    -relief raised \
    -text "Filename:"
  catch ".fsBox.file.labelfile config $tmpMessageOpt"

  entry .fsBox.file.file \
    -relief raised
  catch ".fsBox.file.file config $tmpMessageOpt"

  .fsBox.file.file delete 0 end
  .fsBox.file.file insert 0 $fsBox(name)
  
  checkbutton .fsBox.pattern.all \
    -offvalue 0 \
    -onvalue 1 \
    -text "Show all files" \
    -variable fsBox(all) \
    -command {
      global fsBox
      FSBoxFSShow [.fsBox.path.path get] \
        [.fsBox.pattern.pattern get] $fsBox(all)}
  catch ".fsBox.pattern.all config $tmpButtonOpt"

  FSBoxFSShow $fsBox(internalPath) $fsBox(pattern) $fsBox(all)

  # bindings
  bind .fsBox.files.files <Double-Button-1> "
    FSBoxFSFileSelectDouble %W $fsBox(showPixmap) \{$fsBoxActionOk\} %y"
  bind .fsBox.files.files <ButtonPress-1> "
    FSBoxFSFileSelect %W $fsBox(showPixmap) %y"
  bind .fsBox.files.files <Button1-Motion> "
    FSBoxFSFileSelect %W $fsBox(showPixmap) %y"
  bind .fsBox.files.files <Shift-Button1-Motion> "
    FSBoxFSFileSelect %W $fsBox(showPixmap) %y"
  bind .fsBox.files.files <Shift-ButtonPress-1> "
    FSBoxFSFileSelect %W $fsBox(showPixmap) %y"

  bind .fsBox.path.path <Tab> {
    FSBoxFSNameComplete path}
  bind .fsBox.path.path <Return> {
    global tkVersion
    global fsBox
    FSBoxFSShow [.fsBox.path.path get] \
      [.fsBox.pattern.pattern get] $fsBox(all)
    FSBoxFSInsertPath
    if {$tkVersion >= 3.0} {
      .fsBox.file.file icursor end
    } {
      .fsBox.file.file cursor end
    }
    focus .fsBox.file.file}
  catch "bind .fsBox.path.path <Up> {}"
  bind .fsBox.path.path <Down> {
    global tkVersion
    if {$tkVersion >= 3.0} {
      .fsBox.file.file icursor end
    } {
      .fsBox.file.file cursor end
    }
    focus .fsBox.file.file}

  bind .fsBox.file.file <Tab> {
    FSBoxFSNameComplete file}
  bind .fsBox.file.file <Return> "
    global fsBox
    set fsBox(name) \[.fsBox.file.file get\]
    if {$fsBox(showPixmap)} {
      set fsBox(path) @\[.fsBox.path.path get\]
    } {
      set fsBox(path) \[.fsBox.path.path get\]
    }
    set fsBox(internalPath) \[.fsBox.path.path get\]
    $fsBoxActionOk
    if {\"\[info commands XFDestroy\]\" != \"\"} {
      catch {XFDestroy .fsBox}
    } {
      catch {destroy .fsBox}
    }"
  bind .fsBox.file.file <Up> {
    global tkVersion
    if {$tkVersion >= 3.0} {
      .fsBox.path.path icursor end
    } {
      .fsBox.path.path cursor end
    }
    focus .fsBox.path.path}
  bind .fsBox.file.file <Down> {
    global tkVersion
    if {$tkVersion >= 3.0} {
      .fsBox.pattern.pattern icursor end
    } {
      .fsBox.pattern.pattern cursor end
    }
    focus .fsBox.pattern.pattern}

  bind .fsBox.pattern.pattern <Return> {
    global fsBox
    FSBoxFSShow [.fsBox.path.path get] \
      [.fsBox.pattern.pattern get] $fsBox(all)}
  bind .fsBox.pattern.pattern <Up> {
    global tkVersion
    if {$tkVersion >= 3.0} {
      .fsBox.file.file icursor end
    } {
      .fsBox.file.file cursor end
    }
    focus .fsBox.file.file}
  catch "bind .fsBox.pattern.pattern <Down> {}"

  # packing
  pack append .fsBox.files \
              .fsBox.files.vscroll "$fsBox(scrollSide) filly" \
              .fsBox.files.hscroll {bottom fillx} \
              .fsBox.files.files {left fill expand}
  pack append .fsBox.file \
              .fsBox.file.labelfile {left} \
              .fsBox.file.file {left fill expand}
  pack append .fsBox.frame1 \
              .fsBox.frame1.ok {left fill expand} \
              .fsBox.frame1.rescan {left fill expand} \
              .fsBox.frame1.cancel {left fill expand}
  pack append .fsBox.path.paths \
              .fsBox.path.paths.paths {left}
  pack append .fsBox.pattern.patterns \
              .fsBox.pattern.patterns.patterns {left}
  pack append .fsBox.path \
              .fsBox.path.paths {left} \
              .fsBox.path.path {left fill expand}
  pack append .fsBox.pattern \
              .fsBox.pattern.patterns {left} \
              .fsBox.pattern.all {right fill} \
              .fsBox.pattern.pattern {left fill expand}
  if {$fsBox(showPixmap)} {
    pack append .fsBox.frame2 \
                .fsBox.frame2.scrollbar1 {left filly} \
                .fsBox.frame2.canvas2 {top expand fill} \
                .fsBox.frame2.scrollbar3 {top fillx} 

    pack append .fsBox \
                .fsBox.message1 {top fill} \
                .fsBox.frame1 {bottom fill} \
                .fsBox.pattern {bottom fill} \
                .fsBox.file {bottom fill} \
                .fsBox.path {bottom fill} \
                .fsBox.frame2 {right fill} \
                .fsBox.files {left fill expand}
  } {
    pack append .fsBox \
                .fsBox.message1 {top fill} \
                .fsBox.frame1 {bottom fill} \
                .fsBox.pattern {bottom fill} \
                .fsBox.file {bottom fill} \
                .fsBox.path {bottom fill} \
                .fsBox.files {left fill expand}
  }

  if {"$fsBoxActionOk" == "" && "$fsBoxActionCancel" == ""} {
    # wait for the box to be destroyed
    update idletask
    grab .fsBox
    tkwait window .fsBox

    if {"[string trim $fsBox(path)]" != "" ||
        "[string trim $fsBox(name)]" != ""} {
      if {"[string trimleft [string trim $fsBox(name)] /]" == ""} {
        return [string trimright [string trim $fsBox(path)] /]
      } {
        return [string trimright [string trim $fsBox(path)] /]/[string trimleft [string trim $fsBox(name)] /]
      }
    }
  }
}

##########
# Procedure: FSBoxFSFileSelect
# Description: select file name
# Arguments: fsBoxW - the widget
#            fsBoxShowPixmap - show pixmaps
#            fsBoxY - the y position in the listbox
# Returns: none
# Sideeffects: none
##########
proc FSBoxFSFileSelect {fsBoxW fsBoxShowPixmap fsBoxY} {# xf ignore me 6
  global fsBox

  FSBoxBindSelectOne $fsBoxW $fsBoxY
  set fsBoxNearest [$fsBoxW nearest $fsBoxY]
  if {$fsBoxNearest >= 0} {
    set fsBoxTmpEntry [$fsBoxW get $fsBoxNearest]
    if {"[string index $fsBoxTmpEntry \
          [expr [string length $fsBoxTmpEntry]-1]]" == "/" ||
        "[string index $fsBoxTmpEntry \
          [expr [string length $fsBoxTmpEntry]-1]]" == "@"} {
      set fsBoxFileName [string range $fsBoxTmpEntry 0 \
            [expr [string length $fsBoxTmpEntry]-2]]
      if {![IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]] &&
          ![IsASymlink [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
        set fsBoxFileName $fsBoxTmpEntry
      }
    } {
      if {"[string index $fsBoxTmpEntry \
            [expr [string length $fsBoxTmpEntry]-1]]" == "*"} {
        set fsBoxFileName [string range $fsBoxTmpEntry 0 \
          [expr [string length $fsBoxTmpEntry]-2]]
        if {![file executable $fsBox(internalPath)/$fsBoxFileName]} {
          set fsBoxFileName $fsBoxTmpEntry
        }
      } {
        set fsBoxFileName $fsBoxTmpEntry
      }
    }
    if {![IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
      set fsBox(name) $fsBoxFileName
      .fsBox.file.file delete 0 end
      .fsBox.file.file insert 0 $fsBox(name)
      if {$fsBoxShowPixmap} {
        catch ".fsBox.frame2.canvas2 itemconfigure currentBitmap -bitmap \"@$fsBox(internalPath)/$fsBox(name)\""
      }
    }
  }
}

##########
# Procedure: FSBoxFSFileSelectDouble
# Description: select file when double clicked
# Arguments: fsBoxW - the widget
#            fsBoxShowPixmap - show pixmaps
#            fsBoxAction - the action bound to the ok button
#            fsBoxY - the y position in the listbox
# Returns: none
# Sideeffects: none
##########
proc FSBoxFSFileSelectDouble {fsBoxW fsBoxShowPixmap fsBoxAction fsBoxY} {# xf ignore me 6
  global fsBox

  FSBoxBindSelectOne $fsBoxW $fsBoxY
  set fsBoxNearest [$fsBoxW nearest $fsBoxY]
  if {$fsBoxNearest >= 0} {
    set fsBoxTmpEntry [$fsBoxW get $fsBoxNearest]
    if {"$fsBoxTmpEntry" == "../"} {
      set fsBoxTmpEntry [string trimright [string trim $fsBox(internalPath)] "@/"]
      if {"$fsBoxTmpEntry" == ""} {
        return
      }
      FSBoxFSShow [file dirname $fsBoxTmpEntry] \
        [.fsBox.pattern.pattern get] $fsBox(all)
      .fsBox.path.path delete 0 end
      .fsBox.path.path insert 0 $fsBox(internalPath)
    } {
      if {"[string index $fsBoxTmpEntry \
            [expr [string length $fsBoxTmpEntry]-1]]" == "/" ||
          "[string index $fsBoxTmpEntry \
            [expr [string length $fsBoxTmpEntry]-1]]" == "@"} {
        set fsBoxFileName [string range $fsBoxTmpEntry 0 \
              [expr [string length $fsBoxTmpEntry]-2]]
        if {![IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]] &&
            ![IsASymlink [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
          set fsBoxFileName $fsBoxTmpEntry
        }
      } {
        if {"[string index $fsBoxTmpEntry \
              [expr [string length $fsBoxTmpEntry]-1]]" == "*"} {
          set fsBoxFileName [string range $fsBoxTmpEntry 0 \
                [expr [string length $fsBoxTmpEntry]-2]]
          if {![file executable $fsBox(internalPath)/$fsBoxFileName]} {
            set fsBoxFileName $fsBoxTmpEntry
          }
        } {
          set fsBoxFileName $fsBoxTmpEntry
        }
      }
      if {[IsADir [string trimright $fsBox(internalPath)/$fsBoxFileName @]]} {
        set fsBox(internalPath) "[string trimright $fsBox(internalPath) {/@}]/$fsBoxFileName"
        FSBoxFSShow $fsBox(internalPath) \
          [.fsBox.pattern.pattern get] $fsBox(all)
        .fsBox.path.path delete 0 end
        .fsBox.path.path insert 0 $fsBox(internalPath)
      } {
        set fsBox(name) $fsBoxFileName
        if {$fsBoxShowPixmap} {
          set fsBox(path) @$fsBox(internalPath)
        } {
          set fsBox(path) $fsBox(internalPath)
        }
        if {"$fsBoxAction" != ""} {
          eval "global fsBox; $fsBoxAction"
        }
        if {"[info commands XFDestroy]" != ""} {
          catch {XFDestroy .fsBox}
        } {
          catch {destroy .fsBox}
        }
      }
    }
  }
}

##########
# Procedure: FSBoxFSInsertPath
# Description: insert current pathname into menu
# Arguments: none
# Returns: none
# Sideeffects: none
##########
proc FSBoxFSInsertPath {} {# xf ignore me 6
  global fsBox

  set fsBoxLast [.fsBox.path.paths.paths.menu index last]
  set fsBoxNewEntry [string trimright [.fsBox.path.path get] "/@"]
  for {set fsBoxCounter 0} {$fsBoxCounter <= $fsBoxLast} {incr fsBoxCounter 1} {
    if {"$fsBoxNewEntry" == \
          "[lindex [.fsBox.path.paths.paths.menu entryconfigure \
                    $fsBoxCounter -label] 4]"} {
      return
    }
  }
  if {$fsBoxLast < 9} {
    .fsBox.path.paths.paths.menu add command \
      -label "$fsBoxNewEntry" \
      -command "
        global fsBox
        FSBoxFSShow $fsBoxNewEntry \
          \[.fsBox.pattern.pattern get\] \$fsBox(all)
        .fsBox.path.path delete 0 end
        .fsBox.path.path insert 0 $fsBoxNewEntry"
  } {
    for {set fsBoxCounter 0} {$fsBoxCounter < $fsBoxLast} {incr fsBoxCounter 1} {
      .fsBox.path.paths.paths.menu entryconfigure \
        $fsBoxCounter -label \
          [lindex [.fsBox.path.paths.paths.menu entryconfigure \
            [expr $fsBoxCounter+1] -label] 4]
      .fsBox.path.paths.paths.menu entryconfigure $fsBoxCounter \
        -command "
          global fsBox
          FSBoxFSShow [lindex [.fsBox.path.paths.paths.menu entryconfigure \
            [expr $fsBoxCounter+1] -label] 4] \
            \[.fsBox.pattern.pattern get\] \$fsBox(all)
          .fsBox.path.path delete 0 end
          .fsBox.path.path insert 0 [lindex \
            [.fsBox.path.paths.paths.menu entryconfigure \
              [expr $fsBoxCounter+1] -label] 4]"
    }
    .fsBox.path.paths.paths.menu entryconfigure $fsBoxLast \
      -label "$fsBoxNewEntry"
    .fsBox.path.paths.paths.menu entryconfigure $fsBoxCounter \
      -command "
        global fsBox
        FSBoxFSShow \[.fsBox.path.path get\] \
          \[.fsBox.pattern.pattern get\] \$fsBox(all)
        .fsBox.path.path delete 0 end
        .fsBox.path.path insert 0 $fsBoxNewEntry"
  }
}

##########
# Procedure: FSBoxFSNameComplete
# Description: perform name completion for fs box
# Arguments: fsBoxType - the type we want to complete (path or file)
# Returns: none
# Sideeffects: none
##########
proc FSBoxFSNameComplete {fsBoxType} {# xf ignore me 6
  global tkVersion
  global fsBox

  set fsBoxNewFile ""
  if {"$fsBoxType" == "path"} {
    set fsBoxDirName [file dirname [.fsBox.path.path get]]
    set fsBoxFileName [file tail [.fsBox.path.path get]]
  } {
    set fsBoxDirName [file dirname [.fsBox.path.path get]/]
    set fsBoxFileName [file tail [.fsBox.file.file get]]
  }

  set fsBoxNewFile ""
  if {[IsADir [string trimright $fsBoxDirName @]]} {
    catch "glob -nocomplain $fsBoxDirName/${fsBoxFileName}*" fsBoxResult
    foreach fsBoxCounter $fsBoxResult {
      if {"$fsBoxNewFile" == ""} {
        set fsBoxNewFile [file tail $fsBoxCounter]
      } {
        if {"[string index [file tail $fsBoxCounter] 0]" !=
            "[string index $fsBoxNewFile 0]"} {
          set fsBoxNewFile ""
          break
        }
        set fsBoxCounter1 0
        set fsBoxTmpFile1 $fsBoxNewFile
        set fsBoxTmpFile2 [file tail $fsBoxCounter]
        set fsBoxLength1 [string length $fsBoxTmpFile1]
        set fsBoxLength2 [string length $fsBoxTmpFile2]
        set fsBoxNewFile ""
        if {$fsBoxLength1 > $fsBoxLength2} {
          set fsBoxLength1 $fsBoxLength2
        }
        while {$fsBoxCounter1 < $fsBoxLength1} {
          if {"[string index $fsBoxTmpFile1 $fsBoxCounter1]" == \
                "[string index $fsBoxTmpFile2 $fsBoxCounter1]"} {
            append fsBoxNewFile [string index $fsBoxTmpFile1 $fsBoxCounter1]
          } {
            break
          }
          incr fsBoxCounter1 1
        }
      }
    }
  }
  if {"$fsBoxNewFile" != ""} {
    if {[IsADir [string trimright $fsBoxDirName/$fsBoxNewFile @]] ||
        ![IsAFile [string trimright $fsBoxDirName/$fsBoxNewFile @]]} {
      if {[IsADir [string trimright $fsBoxDirName/$fsBoxNewFile @]]} {
        if {"$fsBoxDirName" == "/"} {
          .fsBox.path.path delete 0 end
          .fsBox.path.path insert 0 "/[string trimright [string trim $fsBoxNewFile /] @]/"
        } {
          .fsBox.path.path delete 0 end
          .fsBox.path.path insert 0 "[string trimright $fsBoxDirName /]/[string trimright [string trim $fsBoxNewFile /] @]/"
        }
        FSBoxFSShow [.fsBox.path.path get] \
          [.fsBox.pattern.pattern get] $fsBox(all)
        FSBoxFSInsertPath
      } {
        .fsBox.path.path delete 0 end
        .fsBox.path.path insert 0 "[string trimright $fsBoxDirName /]/[string trimright [string trim $fsBoxNewFile /] @]"
      }
    } {
      .fsBox.path.path delete 0 end
      .fsBox.path.path insert 0 "[string trimright $fsBoxDirName {@/}]/"
      .fsBox.file.file delete 0 end
      .fsBox.file.file insert 0 $fsBoxNewFile
      if {$tkVersion >= 3.0} {
        .fsBox.file.file icursor end
      } {
        .fsBox.file.file cursor end
      }
      focus .fsBox.file.file
    }
  }
}

##########
# Procedure: FSBoxFSShow
# Description: show the file list
# Arguments: fsBoxPath - the path to show
#            fsBoxPattern - selection pattern
#            fsBoxAll - show all files
# Returns: none
# Sideeffects: none
##########
proc FSBoxFSShow {fsBoxPath fsBoxPattern fsBoxAll} {# xf ignore me 6
  global fsBox

  set tmpButtonOpt ""
  if {"$fsBox(activeBackground)" != ""} {
    append tmpButtonOpt "-activebackground \"$fsBox(activeBackground)\" "
  }
  if {"$fsBox(activeForeground)" != ""} {
    append tmpButtonOpt "-activeforeground \"$fsBox(activeForeground)\" "
  }
  if {"$fsBox(background)" != ""} {
    append tmpButtonOpt "-background \"$fsBox(background)\" "
  }
  if {"$fsBox(font)" != ""} {
    append tmpButtonOpt "-font \"$fsBox(font)\" "
  }
  if {"$fsBox(foreground)" != ""} {
    append tmpButtonOpt "-foreground \"$fsBox(foreground)\" "
  }

  set fsBox(pattern) $fsBoxPattern
  if {[file exists $fsBoxPath] && [file readable $fsBoxPath] &&
      [IsADir $fsBoxPath]} {
    set fsBox(internalPath) $fsBoxPath
  } {
    if {[file exists $fsBoxPath] && [file readable $fsBoxPath] &&
        [IsAFile $fsBoxPath]} {
      set fsBox(internalPath) [file dirname $fsBoxPath]
      .fsBox.file.file delete 0 end
      .fsBox.file.file insert 0 [file tail $fsBoxPath]
      set fsBoxPath $fsBox(internalPath)
    } {
      while {"$fsBoxPath" != "" && "$fsBoxPath" != "/" &&
             ![file isdirectory $fsBoxPath]} {
        set fsBox(internalPath) [file dirname $fsBoxPath]
         set fsBoxPath $fsBox(internalPath)
      }
    }
  }
  if {"$fsBoxPath" == ""} {
    set fsBoxPath "/"
    set fsBox(internalPath) "/"
  }
  .fsBox.path.path delete 0 end
  .fsBox.path.path insert 0 $fsBox(internalPath)

  if {[.fsBox.files.files size] > 0} {
    .fsBox.files.files delete 0 end
  }
  if {$fsBoxAll} {
    if {[catch "exec ls -F -a $fsBoxPath" fsBoxResult]} {
      puts stderr "$fsBoxResult"
    }
  } {
    if {[catch "exec ls -F $fsBoxPath" fsBoxResult]} {
      puts stderr "$fsBoxResult"
    }
  }
  set fsBoxElementList [lsort $fsBoxResult]

  foreach fsBoxCounter [winfo children .fsBox.pattern.patterns.patterns] {
    if {[string length [info commands XFDestroy]] > 0} {
      catch {XFDestroy $fsBoxCounter}
    } {
      catch {destroy $fsBoxCounter}
    }
  }
  menu .fsBox.pattern.patterns.patterns.menu
  catch ".fsBox.pattern.patterns.patterns.menu config $tmpButtonOpt"

  if {$fsBox(extensions)} {
    .fsBox.pattern.patterns.patterns.menu add command \
      -label "*" \
      -command {
        global fsBox
        set fsBox(pattern) "*"
        .fsBox.pattern.pattern delete 0 end
        .fsBox.pattern.pattern insert 0 $fsBox(pattern)
        FSBoxFSShow [.fsBox.path.path get] $fsBox(pattern) \
          $fsBox(all)}
  }

  if {"$fsBoxPath" != "/"} {
    .fsBox.files.files insert end "../"
  }
  foreach fsBoxCounter $fsBoxElementList {
    if {[string match $fsBoxPattern $fsBoxCounter] ||
        [IsADir [string trimright $fsBoxPath/$fsBoxCounter "/@"]]} {
      if {"$fsBoxCounter" != "../" &&
          "$fsBoxCounter" != "./"} {
        .fsBox.files.files insert end $fsBoxCounter
      }
    }

    if {$fsBox(extensions)} {
      catch "file rootname $fsBoxCounter" fsBoxRootName
      catch "file extension $fsBoxCounter" fsBoxExtension
      set fsBoxExtension [string trimright $fsBoxExtension "/*@"]
      if {"$fsBoxExtension" != "" && "$fsBoxRootName" != ""} {
        set fsBoxInsert 1
        set fsBoxLast [.fsBox.pattern.patterns.patterns.menu index last]
        for {set fsBoxCounter1 0} {$fsBoxCounter1 <= $fsBoxLast} {incr fsBoxCounter1 1} {
          if {"*$fsBoxExtension" == \
                "[lindex [.fsBox.pattern.patterns.patterns.menu entryconfigure \
                        $fsBoxCounter1 -label] 4]"} {
            set fsBoxInsert 0
          }
        }
	if {$fsBoxInsert} {
          .fsBox.pattern.patterns.patterns.menu add command \
            -label "*$fsBoxExtension" \
            -command "
              global fsBox
              set fsBox(pattern) \"*$fsBoxExtension\"
              .fsBox.pattern.pattern delete 0 end
              .fsBox.pattern.pattern insert 0 \$fsBox(pattern)
              FSBoxFSShow \[.fsBox.path.path get\] \$fsBox(pattern) \
                \$fsBox(all)"
        }
      }
    }
  }
  if {$fsBox(extensions)} {
    .fsBox.pattern.patterns.patterns.menu add separator
  }
  if {$fsBox(extensions) || 
      "[.fsBox.pattern.patterns.patterns.menu index last]" == "none"} {
    .fsBox.pattern.patterns.patterns.menu add checkbutton \
      -label "Scan extensions" \
      -variable "fsBox(extensions)" \
      -command {
        global fsBox
        FSBoxFSShow [.fsBox.path.path get] \
          [.fsBox.pattern.pattern get] $fsBox(all)}
  }
}

##########
# Procedure: FSBoxBindSelectOne
# Description: action to select the current list item
# Arguments: fsBoxW - the widget
#            fsBoxY - the y position in the listbox
# Returns: none
# Sideeffects: none
##########
proc FSBoxBindSelectOne {fsBoxW fsBoxY} {# xf ignore me 6

  set fsBoxNearest [$fsBoxW nearest $fsBoxY]
  if {$fsBoxNearest >= 0} {
    $fsBoxW selection anchor $fsBoxNearest
    $fsBoxW selection set anchor $fsBoxNearest
  }
}

proc IsADir {pathName} {# xf ignore me 5
##########
# Procedure: IsADir
# Description: check if name is a directory (including symbolic links)
# Arguments: pathName - the path to check
# Returns: 1 if its a directory, otherwise 0
# Sideeffects: none
##########

  if {[file isdirectory $pathName]} {
    return 1
  } {
    catch "file type $pathName" fileType
    if {"$fileType" == "link"} {
      if {[catch "file readlink $pathName" linkName]} {
        return 0
      }
      catch "file type $linkName" fileType
      while {"$fileType" == "link"} {
        if {[catch "file readlink $linkName" linkName]} {
          return 0
        }
        catch "file type $linkName" fileType
      }
      return [file isdirectory $linkName]
    }
  }
  return 0
}

proc IsAFile {fileName} {# xf ignore me 5
##########
# Procedure: IsAFile
# Description: check if filename is a file (including symbolic links)
# Arguments: fileName - the filename to check
# Returns: 1 if its a file, otherwise 0
# Sideeffects: none
##########

  if {[file isfile $fileName]} {
    return 1
  } {
    catch "file type $fileName" fileType
    if {"$fileType" == "link"} {
      if {[catch "file readlink $fileName" linkName]} {
        return 0
      }
      catch "file type $linkName" fileType
      while {"$fileType" == "link"} {
        if {[catch "file readlink $linkName" linkName]} {
          return 0
        }
        catch "file type $linkName" fileType
      }
      return [file isfile $linkName]
    }
  }
  return 0
}

proc IsASymlink {fileName} {# xf ignore me 5
##########
# Procedure: IsASymlink
# Description: check if filename is a symbolic link
# Arguments: fileName - the path/filename to check
# Returns: none
# Sideeffects: none
##########

  catch "file type $fileName" fileType
  if {"$fileType" == "link"} {
    return 1
  }
  return 0
}

# eof

