import sys
import cv2

from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QPushButton, QFileDialog, QLabel, QGraphicsView, QGraphicsScene
from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout
from PyQt5.QtGui import QPixmap, QImage, QPainter, QPen, QColor, QBrush
from PyQt5.QtCore import QPointF, QRectF, Qt, QTimer


class GraphicView(QGraphicsView):
    def __init__(self, parent):
        super().__init__(parent)
        self.parent = parent
        self.scene = QGraphicsScene()        
        self.setScene(self.scene)

        self.dimensions = (1280,720)
        self.pixmapItem = self.scene.addPixmap(QPixmap(*self.dimensions))

        self.timer = QTimer()
        self.timer.timeout.connect(self.display)

        self.items = {}
        self.items['rect'] = []

        self.start = QPointF()
        self.end = QPointF()

        self.pen = QPen(QColor(0,0,0), 3)
 
        # self.setRenderHint(QPainter.HighQualityAntialiasing)

        self.timer.start()
    
    def display(self):
        _, frame = self.parent.cam.read()
        # self.scene.addPixmap(self.array_to_pixmap(frame))
        self.pixmapItem.setPixmap(self.array_to_pixmap(frame))
        self.setScene(self.scene)        

    def array_to_pixmap(self, array):
        h, w, _ = array.shape
        # print(h, w)
        qimg = QImage(array.data, w, h, w*3, QImage.Format_RGB888)
        qpixmap = QPixmap(qimg)

        return qpixmap
    
    def closeEvent(self, e):
        self.timer.stop()

    def moveEvent(self, e):
        rect = QRectF(self.rect())
        rect.adjust(0,0,-2,-2)
        self.scene.setSceneRect(rect)

    def mousePressEvent(self, e):
        if e.button() == Qt.LeftButton:
            self.start = e.pos()
            self.end = e.pos()
            
            rect = QRectF(self.start, self.end)
            self.items['rect'].append(self.scene.addRect(rect, self.pen))

        if e.button() == Qt.RightButton:
            if self.scene.items():
                self.scene.removeItem(self.items['rect'][-1])
                del(self.items['rect'][-1])

    def mouseMoveEvent(self, e):  
        if e.buttons() & Qt.LeftButton:           
            self.end = e.pos()

            if self.scene.items():
                self.scene.removeItem(self.items['rect'][-1])
                del(self.items['rect'][-1])

            rect = QRectF(self.start, self.end)
            self.items['rect'].append(self.scene.addRect(rect, self.pen))
    
    def mouseReleaseEvent(self, e):
        if e.button() == Qt.LeftButton:
            if self.scene.items():
                self.scene.removeItem(self.items['rect'][-1])
                del(self.items['rect'][-1])

            rect = QRectF(self.start, self.end)
            self.items['rect'].append(self.scene.addRect(rect, self.pen))

rtsp_url = "rtsp://admin:admin@192.168.3.33:554/1"
class Demo(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initCam()
        self.initUI()
    
    def initCam(self):
        self.cam = cv2.VideoCapture(rtsp_url)
        if self.cam.isOpened():
            print('cam open!')

        # cam setting
        self.width = self.cam.get(cv2.CAP_PROP_FRAME_WIDTH)
        self.height = self.cam.get(cv2.CAP_PROP_FRAME_HEIGHT)
        # self.width /= 2
        # self.height /= 2
        self.cam.set(cv2.CAP_PROP_FRAME_WIDTH, self.width)
        self.cam.set(cv2.CAP_PROP_FRAME_HEIGHT, self.height)

    def initUI(self):
        self.view = GraphicView(self)
        self.view.setFixedSize(self.width, self.height)

        main_layout = QHBoxLayout()
        main_layout.addWidget(self.view)

        widget = QWidget(parent=self)
        widget.setLayout(main_layout)
        self.setCentralWidget(widget)

        self.setFixedSize(self.width + 100, self.height+100)
        
        self.show()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = Demo()
    sys.exit(app.exec_())