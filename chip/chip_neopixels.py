#/usr/bin/python
import time
import spidev
import random
import json
import paho.mqtt.client as paho

spi = spidev.SpiDev(32766,0)
spi.max_speed_hz=3000000
num_leds = 29

print("test")

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


neo = neopixel(num_leds,True)
neo.clear()

def on():
  for Led in neo.Leds:
    Led.set(200,200,200,200)
  neo.refresh()

def off():
  neo.clear()

# def on():
#   for i in range(255):
#     for Led in neo.Leds:
#       Led.set(0,0,0,i)
#     time.sleep(0.001)
#     neo.refresh()

# def off():
#   for i in range(255):
#     for Led in neo.Leds:
#       Led.set(255-i,255-i,255-i,255-i)
#     time.sleep(0.001)
#     neo.refresh()


def on_connect(client, userdata, flags, rc):
    print("connected received with code %d." % (rc))
    client.subscribe("/lights")

def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))
 
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.qos)+"====" + str(msg.payload))
    try:
      root = json.loads(str(msg.payload.decode('utf-8')))
      light_state = root["body"]["result"]["parameters"]["light-state"]; # "on"
      if msg.topic == "/lights" :
        if light_state == "on" :
          print("ON")
          on()
        elif light_state == "off" :
          print("OFF")
          off()
        else:
          off()
          print("NO MATCH")
    except:  # includes simplejson.decoder.JSONDecodeError
        print ('Decoding JSON failed')

if __name__ == "__main__" :

   # client = paho.Client()
   # client.on_connect = on_connect
   # client.on_subscribe = on_subscribe
   # client.on_message = on_message
   # client.connect("mqtt.chris.gunawardena.id.au", 1883)
   # client.loop_forever()



   try:
     while True:
       NewC = GetNewColor()

       t = range(num_leds)

       for i in range(16):
         for  Led in neo.Leds:
            Led.set(0,0,0)
         neo.set(t[i],NewC.R,NewC.G,NewC.B, 255)
         neo.refresh()
         time.sleep(0.05)

   except KeyboardInterrupt:
       pass
