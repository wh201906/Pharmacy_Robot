from model import OcrHandle
from PIL import Image, ImageDraw, ImageFont
import time
ocrhandle = OcrHandle()
path = "/home/hdu/Pharmacy_Robot/roi.jpg"
startTime = time.time()

img = Image.open(path)
img_w, img_h = img.size
print(img_w, img_h)

img = img.convert("RGB")


f = open('/home/hdu/Pharmacy_Robot/ocr.txt', 'w')
print(f.writable())
res = ocrhandle.text_predict(img, img_w//32*32)
for i, r in enumerate(res):
    rect, txt, confidence = r
    f.write(txt+'\n')

f.close()
print(time.time()-startTime)
