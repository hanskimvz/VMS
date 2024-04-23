import requests
import json, base64
import numpy as np
import cv2 as cv

from cgis import cgi

param = {
    "ip": "192.168.3.33",
    "port": 80,
    "username": "admin",
    "password": "admin",
    "language": "ko",
    "token":""
}

def getToken(param) :
    r = requests.post("http://%s:%d%s" %(param['ip'], param['port'], cgi['login']), json={"username":param['username'],"password":param['password']})
    arr = json.loads(r.text)
    if arr['code'] == 20000:
        return arr['data']['token']
    else:
        return arr

param['token'] = getToken(param) 
def getSnapshot(param):
    r = requests.get("http://%s:%d%s" %(param['ip'], param['port'], cgi['snapshot']), headers={'X-Token': param['token']})
    arr = json.loads(r.text)
    img = base64.b64decode(arr['data'])
    return img

def showSnapshot(snapshot) :
    nparr = np.frombuffer(snapshot, np.uint8)
    img = cv.imdecode(nparr,cv.IMREAD_UNCHANGED)  
    cv.imshow("image", img)
    cv.waitKey(0)
    cv.destroyAllWindows()


snapshot = getSnapshot(param)
# showSnapshot(snapshot)
nparr = np.frombuffer(snapshot, np.uint8)
img = cv.imdecode(nparr,cv.IMREAD_UNCHANGED) 
h, w, c = img.shape
# hist = cv.calcHist([img], [0], None, [256], [0,255])

print (h, w, c)
x0,y0,x1,y1 = 1600,400,1900,640
if y1 > h:
    y1 = h
if x1 > w:
    x1 = w
# x0,y0,x1,y1 = 0,0,w,h
cv.imshow("image", img[y0:y1,x0:x1])
hist = cv.calcHist([img[y0:y1,x0:x1]], [0], None, [256], [0,255])

x0,y0,x1,y1 = 1000,800,2000,1500
if y1 > h:
    y1 = h
if x1 > w:
    x1 = w
cv.imshow("image2", img[y0:y1,x0:x1])

cv.waitKey(0)
cv.destroyAllWindows()

print(hist)


# url = "http://192.168.3.33/api/v1/media/settings?query=video_codec"
# r = requests.get(url, headers=headers)

# print (r.text)