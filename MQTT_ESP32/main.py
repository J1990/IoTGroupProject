# "/dev/cu.SLAB_USBtoUART" is my serial port (Similar to com4 or com5 etc)

#You've to save the Private Key and Certificate in the ROOT of ESP32
#For windows you can use putty or in UNIX based system, below (I used the below one),

#UNIX
# Open Terminal
# pip install esptool
# pip install adafruit-ampy
# ampy port -p <Your Serial Port Here> ls     // This will show all your files in the ESP32 ROOT 
# cd <DIRECTORY WHERE ALL YOUR CERTIFICATES ARE THERE>
# ampy port -p <Your Serial Port Here> put <--------.pem.crt>
# ampy port -p <Your Serial Port Here> put <--------.pem.key>
# You can use the same format to put any other file.
# I used uPyCraft to run this.


from time import sleep
import network
import time

led = Pin(2, Pin.OUT)

sta_if = network.WLAN(network.STA_IF)
ap_if = network.WLAN(network.AP_IF)

sta_if.active(True)
sta_if.connect('WIFI_SSID', 'WIFI_PASSWORD')

#Change the certificate files as necessary and host to ARN
certificate_file = "\b7c528a50a-certificate.pem.crt" # Certificate File name in ROOT of ESP32
private_key_file = "\b7c528a50a-private.pem.key" # Private Key File Name in ROOT of ESP32
mqtt_client_id = "ESP32" #Thing Name you created in AWS IOT Core
mqtt_port = 8883 #Default Port number is 8883
mqtt_topic = "ESP32/Publish" #Thing/Policy (Policy name you created in AWS)
mqtt_host = "a1dx1mfj6ybux0-ats.iot.us-east-2.amazonaws.com" #HTTPS REST API Endpoint in AWS found under Interact Tab.
mqtt_client = None

with open(private_key_file, "r") as f:
 private_key = f.read() 
with open(certificate_file, "r") as f:
 certificate = f.read()


from umqtt.robust import MQTTClient #This is using Robust Client
mqtt_client = MQTTClient(client_id=mqtt_client_id, server=mqtt_host, port=mqtt_port, keepalive=4000, ssl=True,ssl_params={"cert":certificate, "key":private_key, "server_side":False})

#Same Error with Simple Client as above

#from umqtt.simple import MQTTClient 
# Connect to MQTT broker.
#mqtt_client = MQTTClient( "ESP32",
#                    mqtt_host,
#                    port = 8883,
#                    keepalive = 10000,
#                    ssl = True,
#                    ssl_params = {
#                      "cert":certificate, "key":private_key,"server_side":False
#                    } )
# mqtt.connect()

print("No error Till Now")
mqtt_client.connect()
print("Error happens in before line")


time.sleep_ms(1000)
while True:
  led.value(not led.value())
  sleep(0.5)
    
# Publish a test MQTT message.
#mqtt.publish( topic = mqtt_topic, msg = 'hello world', qos = 0 )

#You remove the comment from above line and test it using test tab in AWS.
