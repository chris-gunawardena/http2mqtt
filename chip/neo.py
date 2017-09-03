#/usr/bin/python
import time
import spidev
import random

NUL_LED = 29

spi = spidev.SpiDev(32766,0)
spi.max_speed_hz=3000000

class RGB:
  def __init__(self,R=0,G=0,B=0,W=0):
    self.R=R
    self.G=G
    self.B=B
    self.W=W

  def set(self,R,G,B,W=0):
    self.R=R
    self.G=G
    self.B=B
    self.W=W


class neopixel:
  def __init__(self,NbLeds,WhiteFlag):
    self.NbLeds = NbLeds
    self.WhiteFlag = WhiteFlag
    self.Leds = [ RGB() for i in range(NbLeds)]

  def set(self,Index,R,G,B,W=0):
    self.Leds[Index].R = R
    self.Leds[Index].G = G
    self.Leds[Index].B = B
    self.Leds[Index].W = W

  def clear(self):
    spi.xfer2([0]* 50 + [0x88] * 128)


  def fillColor(self, Colors):
    stickColor=[]    
    temp =0
     
    for i in range(0,8,2):
       if (Colors & 128) == 128:
          temp = 0b11000000
       else:
          temp = 0b10000000
       if (Colors & 64) == 64:
          temp = temp | 0b1100
       else:
          temp = temp | 0b1000
       stickColor.append(temp)
       Colors = Colors << 2
    return stickColor

  def refresh(self):
    stick=[0] * 50
    for Led in self.Leds:
       stick= stick + self.fillColor(Led.G)
       stick= stick + self.fillColor(Led.R)
       stick= stick + self.fillColor(Led.B)
       #do we have a white LED 
       if self.WhiteFlag:
        stick = stick + self.fillColor(Led.W)
    spi.xfer2(stick,3000000,0,8)


def GetNewColor():
  while True:
    R = int(random.random() * 256)
    if R > 255 : 
      continue
    G = int(random.random() * 256)
    if G > 255 : 
      continue
    B = int(random.random() * 256)
    if B > 255 : 
      continue
    if (R + G + B ) < 128 :
      continue;
    return RGB(R,G,B)

if __name__ == "__main__" :
   neo = neopixel(NUL_LED,True)
   neo.clear()

   try:
     while True:

       NewC = RGB(50,50,50)
       for i in range(NUL_LED):
         neo.set(i,NewC.R,NewC.G,NewC.B)
         neo.refresh()
         time.sleep(0.05)

       NewC = RGB(0,0,0)
       for i in range(NUL_LED):
         neo.set(i,NewC.R,NewC.G,NewC.B)
         neo.refresh()
         time.sleep(0.05)

   except KeyboardInterrupt:
       pass

   neo.clear()





