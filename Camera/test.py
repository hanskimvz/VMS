# from PyQt5 import QtCore, QtGui, QtWidgets

a= [1,2,3,4,5,6,7,8,90]
b=['a','b','c']
print (list(zip(b,[a])))


exit()
from PyQt5.QtWidgets import *
from PyQt5 import uic
from PyQt5.QtCore import QDate, QTime, QThread, pyqtSignal, QTimer, QPointF, Qt, QRectF
from PyQt5.QtGui import *

import cv2 as cv

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

    def get_frame(self):
        ret, frame = self.parent.cap.read()
        frame = cv.resize(frame, self.dimensions, interpolation=cv.INTER_AREA)
        image = QImage(frame, *self.dimensions, QImage.Format_RGB888).rgbSwapped()
        self.pixmapItem.setPixmap(QPixmap.fromImage(image))

    def mousePressEvent(self, event):
        print(event)
        if self.itemAt(event.scenePos(), QTransform()) is None:
            self._current_rect_item = QGraphicsRectItem()
            self._current_rect_item.setPen(Qt.red)
            self._current_rect_item.setFlag(QGraphicsItem.ItemIsMovable, True)
            self.addItem(self._current_rect_item)
            self._start = event.scenePos()
            r = QRectF(self._start, self._start)
            self._current_rect_item.setRect(r)
        super(GraphicsScene, self).mousePressEvent(event)

    def mouseMoveEvent(self, event):
        if self._current_rect_item is not None:
            r = QRectF(self._start, event.scenePos()).normalized()
            self._current_rect_item.setRect(r)
        super(GraphicsScene, self).mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        self._current_rect_item = None
        super(GraphicsScene, self).mouseReleaseEvent(event)

        

rtsp_url = "rtsp://admin:admin@192.168.3.33:554/1"
class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)
        
        self.cap = cv.VideoCapture(rtsp_url)

        self.scene =GraphicsScene(self)
        self.view = QGraphicsView(self.scene)
        self.setCentralWidget(self.view)

        

        # self.scene = GraphicsScene(self)
        # self.dimensions = (288,199)
        # self.pixmapItem = self.scene.addPixmap(QPixmap(*self.dimensions))








       


if __name__ == '__main__':
    import sys

    app = QApplication(sys.argv)
    w = MainWindow()
    w.resize(640, 480)
    w.show()
    sys.exit(app.exec_())