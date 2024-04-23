from PyQt5.QtGui import QMouseEvent
from place_query_data import *
from update_data import *


print (locale.getlocale())
print(locale.getdefaultlocale())

locale_act = locale.getdefaultlocale()[0]
locale_act="EN"
if (locale_act == "ko_KR") and os.path.isfile("Camera/device_manager_ko_KR.ui"):
    form_class = uic.loadUiType("Camera/device_manager_ko_KR.ui")[0]    

elif (locale_act == "zh_CN") and os.path.isfile("Camera/device_manager_zh_CN.ui"):
    form_class = uic.loadUiType("Camera/device_manager_zh_CN.ui")[0]    

else:
    form_class = uic.loadUiType("Camera/device_manager.ui")[0]

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

        self.timer = QTimer(self)
        self.timer.setInterval(int(1000/50))
        self.timer.timeout.connect(self.get_frame)
        self.timer.start()

        self.enableZoneEdit = False

    def get_frame(self):
        _, frame = self.parent.cap.read()
        frame = cv.resize(frame, self.dimensions, interpolation=cv.INTER_AREA)
        image = QImage(frame, *self.dimensions, QImage.Format_RGB888).rgbSwapped()
        self.pixmapItem.setPixmap(QPixmap.fromImage(image))

    def play_video(self, graphic_view, dims=(0, 0)):
        self.dimensions = dims
        if dims:
            self.dimensions = graphic_view.width()-2, graphic_view.height()-2
        self.setSceneRect(0, 0, self.dimensions[0], self.dimensions[1])
        graphic_view.setScene(self)
        self.timer.start()

    def remove_rectangles(self):
        for item in self.items():
            if isinstance(item, QGraphicsRectItem):
                self.removeItem(item)

    def draw_rectangles(self, canvas_size, zones):
        rect_item = [None,None,None,None]
        colors = [Qt.red, Qt.yellow, Qt.green, Qt.blue]

        for i, rect in enumerate(zones):
            if 'enabled' in rect and rect.get('enabled') == False:
                continue

            x = canvas_size[0] * rect['x'] / 100
            y = canvas_size[1] * rect['y'] / 100
            w = canvas_size[0] * rect['width'] / 100
            h = canvas_size[1] * rect['height'] / 100

            rect_item[i] = QGraphicsRectItem()
            rect_item[i].setRect(x,y,w,h)
            rect_item[i].setFlag(QGraphicsItem.ItemIsMovable, True)
            rect_item[i].setPen(colors[i])
            
            if rect.get('fill'):
                rect_item[i].setBrush(Qt.red)

            self.addItem(rect_item[i])

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton and self.enableZoneEdit:
            self._current_rect_item = QGraphicsRectItem()
            self._current_rect_item.setPen(Qt.red)
            self._current_rect_item.setFlag(QGraphicsItem.ItemIsMovable, True)
            self.addItem(self._current_rect_item)
            self._start = event.scenePos()
            r = QRectF(self._start, self._start)
            self._current_rect_item.setRect(r)

        # if self.itemAt(event.scenePos(), QTransform()) is None:
        #     self._current_rect_item = QGraphicsRectItem()
        #     self._current_rect_item.setPen(Qt.red)
        #     self._current_rect_item.setFlag(QGraphicsItem.ItemIsMovable, True)
        #     self.addItem(self._current_rect_item)
        #     self._start = event.scenePos()
        #     r = QRectF(self._start, self._start)
        #     self._current_rect_item.setRect(r)
        super(GraphicsScene, self).mousePressEvent(event)

    def mouseMoveEvent(self, event):
        if self._current_rect_item is not None:
            r = QRectF(self._start, event.scenePos()).normalized()
            self._current_rect_item.setRect(r)
        super(GraphicsScene, self).mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        self._current_rect_item = None
        super(GraphicsScene, self).mouseReleaseEvent(event)



class Mywindow(QMainWindow, form_class):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.main_index = self.main_tab.currentIndex()
        self.main_objname = self.main_tab.currentWidget().objectName()
        self.main_tab.tabBarClicked.connect(self.handle_tabbar_clicked)

        self.subtabs = [self.network_tab, self.media_tab, self.detection_tab, self.peripherial_tab, self.system_tab]
        self.sub_index = []
        self.sub_objname = []
        for tab in self.subtabs:
            self.sub_index.append(tab.currentIndex())
            self.sub_objname.append(tab.currentWidget().objectName())
            tab.tabBarClicked.connect(self.handle_subtab_clicked)

        # self.btn_network_apply.clicked.connect(self.update_data)
        self.btn_network_apply.clicked.connect(self.menuClickedFunc)        
        self.btn_media_apply.clicked.connect(self.update_data)
        self.btn_detection_apply.clicked.connect(self.update_data)

        self.cap = cv.VideoCapture(param['rtsp_url'])
        self.scene = GraphicsScene(self)

        self.onesec_timer = QTimer(self)
        self.onesec_timer.setInterval(1000)

        self.view_query()

    def update_data(self):
        update_data_act(self)

    def handle_tabbar_clicked(self, index):
        self.main_index = index
        self.main_objname = self.main_tab.widget(index).objectName()
        print("M=%d(%s), S=%d(%s)" %(self.main_index, self.main_objname, self.sub_index[self.main_index], self.sub_objname[self.main_index]))
        self.view_query()
        
    def handle_subtab_clicked(self, index):
        self.sub_index[self.main_index] = index
        self.sub_objname[self.main_index] = self.subtabs[self.main_index].widget(index).objectName()
        print("M=%d(%s), S=%d(%s)" %(self.main_index, self.main_objname, self.sub_index[self.main_index], self.sub_objname[self.main_index]))
        self.view_query()

    def view_query(self):
        self.scene.timer.stop()
        self.onesec_timer.stop()
        self.scene.enableZoneEdit = False

        try:
            self.onesec_timer.timeout.disconnect()
        except:
            pass

        self.scene.remove_rectangles()
        view_query_act(self)
        

    # def get_frame(self):
    #     _, frame = self.cap.read()
    #     frame = cv.resize(frame, self.dimensions, interpolation=cv.INTER_AREA)
    #     image = QImage(frame, *self.dimensions, QImage.Format_RGB888).rgbSwapped()
    #     self.pixmapItem.setPixmap(QPixmap.fromImage(image))


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

    def menuClickedFunc(self):
        # The sender object:
        sender = self.sender()
        # The sender object's name:
        senderName = sender.objectName()
        print (senderName)

import signal
if __name__ == "__main__":
    app = QApplication(sys.argv)
    myWindow = Mywindow()
    myWindow.show()
    app.exec_()
    myWindow.cap.release()
    os.kill(os.getpid(), signal.SIGINT)
