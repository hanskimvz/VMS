import cv2, queue, threading, time

# bufferless VideoCapture
class VideoCapture:

  def __init__(self, name):
    self.cap = cv2.VideoCapture(name)
    self.q = queue.Queue()
    t = threading.Thread(target=self._reader)
    t.daemon = True
    t.start()

  # read frames as soon as they are available, keeping only most recent one
  def _reader(self):
    while True:
      ret, frame = self.cap.read()
      if not ret:
        break
      if not self.q.empty():
        try:
          self.q.get_nowait()   # discard previous (unprocessed) frame
        except queue.Empty:
          pass
      self.q.put(frame)

  def read(self):
    return self.q.get()

url = "rtsp://192.168.3.33/0"
cap = VideoCapture(url)
while True:
  time.sleep(.5)   # simulate time between events
  frame = cap.read()
  cv2.imshow("frame", frame)
  if chr(cv2.waitKey(1)&255) == 'q':
    break
  
sys.exit()

import requests
import sys
import time
import webbrowser
from urllib.parse import urlparse
from subprocess import call 

url = ""
domain = ""
args = sys.argv
if len(args) == 1:
    url = input('Enter the url : ')
else:
    url = args[1]
domain = urlparse(url).netloc
speech = "Sir, website is now online."
while True:
    try:
        res = requests.get(url)
        if res.status_code == requests.codes.ok:
            print("200\n" + domain + " is now online")
            webbrowser.open(url,new=2)
            for i in range(2):
                call(["notify-send",domain,"is now online"])
                call(["espeak",speech]);
                time.sleep(3)
            break
        time.sleep(10)
    except requests.ConnectionError as e:
        print("Check Your internet connection!!")
        sys.exit(1)
    else:
        print("Some Error Occured!!")
        sys.exit(1)
 