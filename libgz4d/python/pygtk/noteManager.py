#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import pygtk
pygtk.require('2.0')
import gtk

class Stroke:
    def __init__(self,x,y,p,time,pixmap,device):
        self.x = x
        self.y = y
        self.time = time
        self.pixmap = pixmap
        self.lastx = x
        self.lasty = y
        self.lastp = p
        self.device = device


    def addPoint(self,widget,x,y,p,device):
        s = int(4*p)
        if self.device == 'eraser':
            olds = int(4*self.lastp)*4
            self.pixmap.draw_rectangle(widget.get_style().white_gc, True, self.lastx-olds/2, self.lasty-olds/2, olds, olds)
            s *= 4
            self.pixmap.draw_rectangle(widget.get_style().mid_gc[0], True, x-s/2, y-s/2, s, s)
            if olds > s:
                s = olds
        else:
            gc = widget.get_style().black_gc
            gc.set_line_attributes(s,gtk.gdk.LINE_SOLID,gtk.gdk.CAP_ROUND,gtk.gdk.JOIN_ROUND)
            self.pixmap.draw_line(gc,self.lastx,self.lasty,x,y)

        if self.lastx < x:
            minx = self.lastx
            maxx = x
        else:
            maxx = self.lastx
            minx = x
        if self.lasty < y:
            miny = self.lasty
            maxy = y
        else:
            maxy = self.lasty
            miny = y
        widget.queue_draw_area(minx-s/2, miny-s/2, maxx+s/2, maxy+s/2)
        self.lastx = x
        self.lasty = y
        self.lastp = p


class NoteManagerWindow:
    def __init__(self):

        self.pixmap = None
        self.lastTime = 0
        self.currentStroke = None
        pixmap = gtk.gdk.Pixmap(None, 1, 1, 1)
        color = gtk.gdk.Color()
        self.blankCursor = gtk.gdk.Cursor(pixmap, pixmap, color, color, 0, 0)


        for d in gtk.gdk.devices_list():
            print d.name
            if d.name in ('stylus','eraser','cursor','Core Pointer'):
                print '^^^^^'
                d.set_mode(gtk.gdk.MODE_WINDOW)

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_name('noteManager')

        vbox = gtk.VBox()
        self.window.add(vbox)
        vbox.show()

        self.window.connect("delete_event", self.delete_event)
        
        self.drawing_area = gtk.DrawingArea()
        vbox.pack_start(self.drawing_area)
        
        self.drawing_area.show()
        
        self.drawing_area.connect('expose_event', self.expose_event)
        self.drawing_area.connect('configure_event', self.configure_event)
        self.drawing_area.connect('motion_notify_event', self.motion_notify_event)
        self.drawing_area.connect('button_press_event', self.button_press_event)
        self.drawing_area.connect('button_release_event', self.button_release_event)
        self.drawing_area.connect('proximity_in_event', self.proximity_in_event)
        self.drawing_area.connect('proximity_out_event', self.proximity_out_event)

        self.drawing_area.set_events(gtk.gdk.EXPOSURE_MASK | gtk.gdk.LEAVE_NOTIFY_MASK | gtk.gdk.BUTTON_PRESS_MASK | gtk.gdk.BUTTON_RELEASE_MASK | gtk.gdk.POINTER_MOTION_MASK | gtk.gdk.POINTER_MOTION_HINT_MASK | gtk.gdk.PROXIMITY_IN_MASK | gtk.gdk.PROXIMITY_OUT_MASK)
        self.drawing_area.set_extension_events(gtk.gdk.EXTENSION_EVENTS_ALL)

        self.window.show()
        gtk.main()

    def delete_event(self, widget, event, data=None):
        gtk.main_quit()
        return False

    def configure_event(self, widget, event):
        x, y, width, height = widget.get_allocation()
        oldPixmap = self.pixmap
        if oldPixmap is not None:
            ow, oh = oldPixmap.get_size()
            width = max(ow,width)
            height = max(oh,height)

        self.pixmap = gtk.gdk.Pixmap(widget.window,width,height)
        self.pixmap.draw_rectangle(widget.get_style().white_gc, True, 0, 0, width, height)

        if oldPixmap is not None:
            self.pixmap.draw_drawable(widget.get_style().black_gc,oldPixmap,0,0,0,0,-1,-1)

        return True

    def expose_event(self, widget, event):
        x , y, width, height = event.area
        widget.window.draw_drawable(widget.get_style().fg_gc[gtk.STATE_NORMAL], self.pixmap, x, y, x, y, width, height)
        return False

    def draw_brush(self, widget, x, y,p):
        rect = (int(x-5*p), int(y-5*p), int(10*p), int(10*p))
        self.pixmap.draw_rectangle(widget.get_style().black_gc, True, rect[0], rect[1], rect[2], rect[3])
        widget.queue_draw_area(rect[0], rect[1], rect[2], rect[3])

    def button_press_event(self, widget, event):
        print event.time,event.device.name,event.button,'press'
        p = event.device.get_axis(event.device.get_state(event.window)[0],gtk.gdk.AXIS_PRESSURE)
        x, y, state = event.window.get_pointer()
        if event.device.name == 'eraser':
            event.window.set_cursor(self.blankCursor)
        if self.pixmap != None:
            self.currentStroke = Stroke(x,y,p,event.time,self.pixmap,event.device.name)
        return True

    def button_release_event(self, widget, event):
        print event.time,event.device.name,event.button,'release'
        self.currentStroke = None
        return True

    def proximity_in_event(self, widget, event):
        print event.time,event.device.name,'in'
        event.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.CROSSHAIR))
        return True

    def proximity_out_event(self, widget, event):
        print event.time,event.device.name,'out'
        event.window.set_cursor(None)
        return True

    def motion_notify_event(self, widget, event):
        s = event.device.get_state(event.window)
        if event.is_hint:
            x, y, state = event.window.get_pointer()
        else:
            x = event.x
            y = event.y
            state = event.state

        p = event.device.get_axis(s[0],gtk.gdk.AXIS_PRESSURE)
        xx = event.device.get_axis(s[0],gtk.gdk.AXIS_X)
        yy = event.device.get_axis(s[0],gtk.gdk.AXIS_Y)

        if self.currentStroke != None:
            self.currentStroke.addPoint(widget, x, y, p, event.device.name)

        return True

if __name__ == "__main__":
        nmw = NoteManagerWindow()

