#!/usr/bin/env python
# gz
# GZhttp
# Provides access to GZ state from a web browser

import gz4d
import sys
import os
import shutil
import wx
import BaseHTTPServer
import thread

# Define notification event for thread completion
EVT_LOG_ID = wx.NewId()

def EVT_LOG(win, func):
    """Define Need Refresh Event."""
    win.Connect(-1, -1, EVT_LOG_ID, func)

class LogEvent(wx.PyEvent):
    """Simple event to carry arbitrary result data."""
    def __init__(self,data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_LOG_ID)
        self.data = data

class GZhttpFrame(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self,parent,-1,title,size=(600,200))

        self.gz = gz4d.Connection()
        self.gz.Connect()
        self.gz.lastRequest = '/'
        
        if self.gz.appdir[-1] == "/":
            self.faviconPath = self.gz.appdir+"gzfav.ico"
        else:
            self.faviconPath = self.gz.appdir+"/gzfav.ico"

        self.server_address = ('', 8080)
        self.httpd = BaseHTTPServer.HTTPServer(self.server_address, GZhttpHandler)
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        self.logText = wx.TextCtrl(self,style = wx.TE_MULTILINE)
        sizer.Add(self.logText, 1, wx.ALL | wx.EXPAND, 2)
        self.SetSizer(sizer)
        
        EVT_LOG(self,self.OnLog)
        
        thread.start_new_thread(self.httpd.serve_forever,())
        self.logText.AppendText("Listening on port " + str(self.server_address[1]) + "...\n")
        
    
    def OnLog(self,evt):
        self.logText.AppendText(evt.data)

class GZState:
    def __init__(self,gz):
        self.gz = gz
                
    def getHead(self):
        ret = """
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>GeoZui4D State</title>
<style type="text/css">
html,body{margin:0;padding:0}
body { font-family: arial,sans-serif;font-size: 0.8em;}
table {background-color: #9CF; border: none;}
th {text-align: left; border: none; background-color: #FFD;}
td {background-color: #FFE; border: none;}
hr {color: #99CCFF;}
div#header     { background: #CCCCFF; position: fixed; top: 0; line-height: 1.2em; height: 1.5em; font-size: 1.2em; padding-left: 1em; width: 100%;}
div#content    { background: white; padding-top: 2em; line-height: 1.4; margin-left: 7em; margin-right: 11em;}
div#navigation { background: #99CCFF; position: fixed; top: 1.5em; width: 4.8em; left: 0; padding-left: .5em; height: 100%; font-size: 1.2em;}
div#extra      { background: #99CCFF; position: fixed; top: 1.5em; width: 10em; right: 0; padding-left: .5em; height: 100%; font-size: 1.2em;}
</style>
</head>
<body>
"""
        return ret
    
    def getTail(self):
        ret = """
        </body>
        </html>
        """
        return ret
        
    def getPage(self,path):
        start = self.getHead()
        nav = """
        <div id="navigation">
        <strong>Contents</strong>
        <a href="/address/">Address</a>
        <a href="/object/">Objects</a>
        </div>
        """

        head = '<div id="header">GeoZui4D State</div>'
        body = '<div id="content"><p> </p></div>\n'
        extra = '<div id="extra"><p> </p></div>'

        print path

        parts = path.strip('/').split('/')

        if parts[0] == 'object':
            head = '<div id="header"><a href="/object/">/object/</a></div>'

            try:
                oid = int(parts[1])
            except IndexError:
                oid = 0
            r = self.gz.SendCommand('gz htmlInfo object '+str(oid))
            if r[0]:
                body = '<div id="content">\n'
                body += r[1]
                body += '</div>\n'
        
        if parts[0] == 'address':
            if len(parts) == 1:
                a = "toplevel"
                head = '<div id="header"><a href="/address/">/address/</a></div>'
            
            if len(parts) > 1:
                a = parts[1]
        
                aparts = a.strip('.').split('.')
                pplink = "/address/"
                head = '<div id="header"><a href="/address/">/address/</a>'
                c = 0
                for c in range(len(aparts)):
                    pplink = '.'.join((pplink,aparts[c]))
                    if c < len(aparts)-1:
                        head += '.<a href="' + pplink + '">' + aparts[c] + '</a>'
                    else:
                        head += '.' + aparts[c]
                head += "</div>"

            if a != None:
                r = self.gz.SendCommand("gz address htmlInfo "+a)
                if r[0]:
                    body = '<div id="content">\n'
                    body += r[1]
                    body += '</div>\n'
                r = self.gz.SendCommand("gz address list "+a)
                if r[0]:
                    p = r[1].split()
                    extra = '<div id="extra">'
                    for i in p:
                        t = i.split('.')
                        extra += '<a href="/address/' + i + '">' + t[-1] + '</a><br>'
                    extra += '<p> </p></div>'

        if parts[0] == 'command':
            if '?' in parts[-1]:
                last,args = parts.pop().split('?',1)
                parts.append(last)
                pairs = args.split('&')
                for p in pairs:
                    k,v = p.split('=',1)
                    newParts = []
                    for i in parts:
                        if i == k:
                            newParts.append(v)
                        else:
                            newParts.append(i)
                    parts = newParts
            print parts
            self.gz.SendCommand(' '.join(parts[1:]))
            return self.getPage(self.gz.lastRequest)

        tail = self.getTail()
        self.gz.lastRequest = path
        return start + head + nav + extra+  body   +tail

class GZhttpHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(self):
        
        global f
        if self.path == "/favicon.ico":
            try:
                icon = open(f.faviconPath,"rb")
            except IOError:
                self.send_error(404,"File not found")
                return None
            self.send_response(200)
            self.send_header("Content-type","image/x-icon")
            self.send_header("Content-Length", str(os.fstat(icon.fileno())[6]))
            self.end_headers()
            shutil.copyfileobj(icon,self.wfile)
            icon.close()
            return

        gzs = GZState(f.gz)
        page = gzs.getPage(self.path)
        
        if len(page) == 0:
            self.send_error(404, "Error")
        else:
            self.send_response(200)
            self.send_head(len(page))
            self.wfile.write(page)
        
    def send_head(self,length):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length",length)
        self.end_headers()
        
    def log_message(self,format,*args):
        global f
        wx.PostEvent(f, LogEvent("%s - - [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format%args)))


class GZhttpApp(wx.App):

    def OnInit(self):
        global f
        f = GZhttpFrame(None,"GeoZui4D web server")
        f.Show()
        self.SetTopWindow(f)
        return True


if __name__ == "__main__":
    app = GZhttpApp()
    app.MainLoop()

