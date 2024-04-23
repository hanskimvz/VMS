
import os,sys
cwd = os.getcwd()
sys.path.append(cwd+"\\Camera")
# print (cwd,sys.path)

from place_query_data import *
from update_data import *
import signal


# print (locale.getlocale())
# print(locale.getdefaultlocale())
locale_act = locale.getdefaultlocale()[0]
locale_act="EN"

ui_filename = cwd + "/Camera/device_manager.ui"
if (locale_act == "ko_KR"):
    ui_filename = cwd + "/Camera/device_manager_ko_KR.ui"
elif (locale_act == "zh_CN"):
    ui_filename = cwd + "/Camera/device_manager_zh_CN.ui"

if os.path.isfile(ui_filename):
    form_class = uic.loadUiType(ui_filename)[0]    




class GraphicsScene(QGraphicsScene,):
    def __init__(self, parent):
        super().__init__(parent)
        # super(GraphicsScene, self).__init__(QRectF(-500, -500, 1000, 1000), parent)
        self.parent = parent

        self._start = QPointF()
        self._end = QPointF()
        self._current_rect_item = None

        self.dimensions = (288,199)
        self.pixmapItem = self.addPixmap(QPixmap(*self.dimensions))    

        self.time_interval =  20
        self.timer = QTimer(self.parent)
        self.timer.setInterval(self.time_interval)
        self.timer.timeout.connect(self.get_frame)
        # self.timer.timeout.connect(self.get_snapshot)
        self.timer.start()

        self.init_rectangles()
        self.enableZoneEdit = False
        
        self.cap = None
        self.init_cam()
        self.play = False
        # th = threading.Thread(target=self.grab_frame, args=(), daemon=True)
        # th.start()

        

    def init_cam(self):
        if self.cap:
            self.cap.release()
            param['token'] = getToken(param) 
            self.cap = None
        for i in range(5):
            try:
                self.cap = cv.VideoCapture(param['rtsp_url'])
            except:
                pass
            time.sleep(1)
            if self.cap:
                break
        
        # self.cap.set(cv.CAP_PROP_BUFFERSIZE, 1)
        # self.cap.set(cv.CAP_PROP_FPS, 5)


    def grab_frame(self):
        while True:
            if self.cap.isOpened():
                self.cap.grab()
            time.sleep(0.1)

    def get_frame(self):
        if not self.play:
            self.cap.grab()
            return
        
        ret, frame = self.cap.read()
        if not ret:
            self.init_cam()
            return
        frame = cv.resize(frame, self.dimensions, interpolation=cv.INTER_AREA)
        image = QImage(frame, *self.dimensions, QImage.Format_RGB888).rgbSwapped()
        self.pixmapItem.setPixmap(QPixmap.fromImage(image))

    def get_snapshot(self):
        snapshot = getSnapshot(param)
        nparr = np.frombuffer(snapshot, np.uint8)
        img = cv.imdecode(nparr,cv.IMREAD_UNCHANGED)         
        frame = cv.resize(img, self.dimensions, interpolation=cv.INTER_AREA)
        image = QImage(frame, *self.dimensions, QImage.Format_RGB888).rgbSwapped()
        self.pixmapItem.setPixmap(QPixmap.fromImage(image))


    def play_video(self, graphic_view, dims=(0, 0)):
        self.play = True
        self.dimensions = dims
        if dims:
            self.dimensions = graphic_view.width()-2, graphic_view.height()-2
        self.setSceneRect(0, 0, self.dimensions[0], self.dimensions[1])
        graphic_view.setScene(self)
        self.timer.start()

    def stop_video(self):
        self.timer.stop()
        try:
            self.cap.release()
        except:
            pass
        self.cap = None

    def init_rectangles(self):
        for item in self.items():
            if isinstance(item, QGraphicsRectItem):
                self.removeItem(item)
        self.zones =[
            {'obj': QGraphicsRectItem(), 'enabled': False, 'color': Qt.red,    'x':0, 'y':0, 'width':0, 'height':0, 'fill':False},
            {'obj': QGraphicsRectItem(), 'enabled': False, 'color': Qt.yellow, 'x':0, 'y':0, 'width':0, 'height':0, 'fill':False},
            {'obj': QGraphicsRectItem(), 'enabled': False, 'color': Qt.green,  'x':0, 'y':0, 'width':0, 'height':0, 'fill':False},
            {'obj': QGraphicsRectItem(), 'enabled': False, 'color': Qt.blue,   'x':0, 'y':0, 'width':0, 'height':0, 'fill':False}
        ]
        for i, zone in enumerate(self.zones):
            self.addItem(self.zones[i]['obj'])
            self.zones[i]['obj'].hide()
        self.current_zone = 0

    def remove_rectangles(self):
        for item in self.items():
            if isinstance(item, QGraphicsRectItem):
                self.removeItem(item)
        for i in range(len(self.zones)):
            self.zones[i]['obj'] = QGraphicsRectItem()
            self.addItem(self.zones[i]['obj'])
            


    def draw_rectangles(self, graphic_view):
        self.enableZoneEdit = True
        self.canvas_size = (graphic_view.width(), graphic_view.height())

        colors = [Qt.red, Qt.yellow, Qt.green, Qt.blue]
        for i, zone in enumerate(self.zones):
            self.zones[i]['obj'].setBrush(colors[i]) if zone.get('fill') else self.zones[i]['obj'].setPen(colors[i])
            self.zones[i]['obj'].hide() if zone.get('enabled') == False else self.zones[i]['obj'].show()            
            x = self.canvas_size[0] * zone['x'] / 100
            y = self.canvas_size[1] * zone['y'] / 100
            w = self.canvas_size[0] * zone['width'] / 100
            h = self.canvas_size[1] * zone['height'] / 100
            self.zones[i]['obj'].setRect(x,y,w,h)


    def mousePressEvent(self, event):
        self.cur = None
        # print (self.current_zone)
        if event.button() == Qt.LeftButton and self.enableZoneEdit and self.zones[self.current_zone].get('enabled'):
            self.zones[self.current_zone]['obj'].hide() if self.zones[self.current_zone].get('enabled') == False else self.zones[self.current_zone]['obj'].show()            
            self._start = event.scenePos()
            r = QRectF(self._start, self._start)
            self.zones[self.current_zone]['obj'].setRect(r)            
            self.cur = True

            # motion detection    

        super(GraphicsScene, self).mousePressEvent(event)

    def mouseMoveEvent(self, event):
        if self.cur :
            r = QRectF(self._start, event.scenePos()).normalized()
            self.zones[self.current_zone]['obj'].setRect(r)

        super(GraphicsScene, self).mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        if self.cur:
            self._last = event.scenePos()
            x0 = self._start.x()
            y0 = self._start.y()
            x1 = self._last.x()
            y1 = self._last.y()

            x = x0 if x0 < x1 else x1
            y = y0 if y0 < y1 else y1
            w = abs(x1 - x0)
            h = abs(y1 - y0)

            self.zones[self.current_zone]['x'] = round(x * 100 / self.canvas_size[0], 2) 
            self.zones[self.current_zone]['y'] = round(y * 100 / self.canvas_size[1] ,2)
            self.zones[self.current_zone]['width']  = round(w * 100 / self.canvas_size[0],2)
            self.zones[self.current_zone]['height'] = round(h * 100 / self.canvas_size[1],2)
            self.cur = None

            if self.parent.main_objname == 'detection_main' and self.parent.sub_objname[self.parent.main_objname] == 'detect_motion':
                self.parent.place_data.motion_event()

            # for z in self.zones:
            #     print(z)
        super(GraphicsScene, self).mouseReleaseEvent(event)



class Mywindow(QMainWindow, form_class):

    def __init__(self):
        super().__init__()
        self.setupUi(self)

        self.main_index = self.main_tab.currentIndex()
        self.main_objname = self.main_tab.currentWidget().objectName()
        self.main_tab.tabBarClicked.connect(self.handle_tabbar_clicked)
        self.sub_tabs = []
        self.sub_objname = {}
        for i in range(self.main_tab.count()):
            w = self.main_tab.widget(i)
            x = w.findChild(QTabWidget)
            self.sub_tabs.append(x)
            self.sub_objname[w.objectName()] = x.currentWidget().objectName()
            x.tabBarClicked.connect(self.handle_subtab_clicked)

        # self.main_tab.setTabEnabled(3, False)
        # self.main_tab.setStyleSheet("QTabBar::tab::disabled {width: 0; height: 0; margin: 0; padding: 0; border: none;} ")
        self.main_tab.setTabVisible(3, False)  # peripherial
        self.network_tab.setTabVisible(1, False) # network.udp
        self.media_tab.setTabVisible(2, False) # media.audio
        

        self.place_data = placeQueryData(self)
        self.upd_data   = updateData(self)
        self.scene      = GraphicsScene(self)

        for b in self.main_tab.findChildren(QPushButton):
            b.clicked.connect(self.update_data)

        self.view_query()



    def handle_tabbar_clicked(self, index):
        self.main_index = index
        self.main_objname = self.main_tab.widget(index).objectName()
        self.view_query()

    def handle_subtab_clicked(self, index):
        self.sub_objname[self.main_objname] = self.sub_tabs[self.main_index].widget(index).objectName()
        # print (self.main_objname,  self.sub_objname[self.main_objname])
        self.view_query()

    def view_query(self):
        # print (self.scene.play)
        self.scene.play = False
        self.scene.init_rectangles()
        self.place_data.view_query_act()


    def update_data(self):
        self.upd_data.update_data_act(self.sender(), self.main_objname, self.sub_objname[self.main_objname] )


    def keyPressEvent(self, e: QKeyEvent) -> None:
        print (e.key())
        if e.key() == 16777216:
            self.close()
            os.kill(os.getpid(), signal.SIGINT)
        # return super().keyPressEvent(e)
    
    # def mouseMoveEvent(self, a0: QMouseEvent) -> None:
    #     return super().mouseMoveEvent(a0)
    # def mousePressEvent(self, e: QMouseEvent) -> None:
    #     print (e.buttons(), e.x, e.y)
    #     return super().mousePressEvent(e)

    # def menuClickedFunc(self):
    #     # The sender object:
    #     sender = self.sender()
    #     # The sender object's name:
    #     senderName = sender.objectName()
    #     print (senderName)



if __name__ == "__main__":
    if not is_online(param['ip'], param['port']):
        print ("device is not online")
        exit(0)    
    
    param['token'] = getToken(param) 

    app = QApplication(sys.argv)
    myWindow = Mywindow()
    myWindow.show()
    app.exec_()
    myWindow.scene.cap.release()
    os.kill(os.getpid(), signal.SIGINT)
