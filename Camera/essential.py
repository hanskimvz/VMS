
from PyQt5.QtWidgets import *
from PyQt5 import uic
from PyQt5.QtCore import QCoreApplication, QMetaObject, QSize, QDate, QTime, QThread, pyqtSignal, QTimer, QPointF, Qt, QRectF, QRect
from PyQt5.QtGui import *

import sys, os, time, datetime
import socket
import requests
import json, base64
import locale
import threading
import cv2 as cv
import numpy as np


from optparse import OptionParser

parser = OptionParser()
parser.add_option("-u", "--user",     dest="username", type="string", help="username")
parser.add_option("-p", "--password", dest="password", type="string", help="password, don't print status messages to stdout")
parser.add_option("-l", "--language", dest="language", type="string", help="language, [ko/eng/ch]")


(options, args) = parser.parse_args()

param = {
    "ip": "192.168.3.33",
    "port": 80,
    "username": "admin",
    "password": "admin",
    "language": "ko",
    "token":"",
    "rtsp_port":554,
    "rtsp_stream": "1"
}


if args:
    param['ip'] = args[0]

if options.username:
    param['username'] = options.username
if options.password:
    param['password'] = options.password


print (param)


cgi_login = "/api/v1/user/login"
cgi_logout= "/api/v1/user/logout"

def getToken(param) :
    r = requests.post("http://%s:%d%s" %(param['ip'], param['port'], cgi_login), json={"username":param['username'],"password":param['password']})
    arr = json.loads(r.text)
    if arr['code'] == 20000:
        return arr['data']['token']
    return False

def is_online(ip, port=80):
    #  if port is not 80 ??
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server = (ip, port)
    s.settimeout(1)
    x =  s.connect_ex(server)
    s.close()
    return not x



param['rtsp_url'] = "rtsp://%s:%s@%s:%d/%s" %(param['username'], param['password'], param['ip'], param['rtsp_port'], str(param["rtsp_stream"]))

def getSnapshot(param):
    r = requests.get("http://%s:%d/api/v1/livestream/capture?stream=1" %(param['ip'], param['port']), headers={'X-Token': param['token']})
    arr = json.loads(r.text)
    img = base64.b64decode(arr['data'])
    return img

def getModel():
    r = requests.get("http://%s:%d/api/v1/system/setting?query=system_info" %(param['ip'], param['port']), headers={'X-Token': param['token']})
    arr = json.loads(r.text)

    return arr['data']['settings']

# class GraphicsScene(QGraphicsScene):
#     def __init__(self, parent=None):


#         super(GraphicsScene, self).__init__(QRectF(-500, -500, 1000, 1000), parent)
#         self._start = QPointF()
#         self._current_rect_item = None

#     def mousePressEvent(self, event):
#         if self.itemAt(event.scenePos(), QTransform()) is None:
#             self._current_rect_item = QGraphicsRectItem()
#             self._current_rect_item.setBrush(Qt.red)
#             self._current_rect_item.setFlag(QGraphicsItem.ItemIsMovable, True)
#             self.addItem(self._current_rect_item)
#             self._start = event.scenePos()
#             r = QRectF(self._start, self._start)
#             self._current_rect_item.setRect(r)
#         super(GraphicsScene, self).mousePressEvent(event)

#     def mouseMoveEvent(self, event):
#         if self._current_rect_item is not None:
#             r = QRectF(self._start, event.scenePos()).normalized()
#             self._current_rect_item.setRect(r)
#         super(GraphicsScene, self).mouseMoveEvent(event)

#     def mouseReleaseEvent(self, event):
#         self._current_rect_item = None
#         super(GraphicsScene, self).mouseReleaseEvent(event)


# def play_video(s, graphic_view, dims = (600,400)):
#     s.dimensions = dims
#     s.scene.setSceneRect(0, 0, s.dimensions[0], s.dimensions[1])
#     graphic_view.setScene(s.scene)
#     s.scene.timer.start()

# print (param['rtsp_url'])
# def has_key(dict_t, key ):
#     for r in dict_t:
#         if r == key:
#             return True
#     return False

# def draw_rectangles(s, graphic_view, zones):
#     for item in s.scene.items():
#         if isinstance(item, QGraphicsRectItem):
#             s.scene.removeItem(item)
#     v_width, v_height = graphic_view.width(), graphic_view.height()
#     rect_item = [None,None,None,None]
#     colors = [Qt.red, Qt.yellow, Qt.green, Qt.blue]
#     for i, rect in enumerate(zones):

#         if 'enabled' in rect and rect.get('enabled') == False:
#             continue

#         # if 'enabled' in rect: 
#         #     print ("has enabled")

#         # color = Qt.red
#         fill = False
#         x = v_width  * rect['x'] / 100
#         y = v_height * rect['y'] / 100
#         w = v_width  * rect['width'] / 100
#         h = v_height * rect['height'] / 100

#         rect_item[i] = QGraphicsRectItem()
#         rect_item[i].setRect(x, y, w, h)
#         rect_item[i].setPen(colors[i])
#         if fill == True:
#             rect_item[i].setBrush(Qt.red)
#         rect_item[i].setFlag(QGraphicsItem.ItemIsMovable, True)
#         s.scene.addItem(rect_item[i])


if __name__ == '__main__':

    print (is_online(param['ip'], param['port']))
    # print (getToken(param))
  

