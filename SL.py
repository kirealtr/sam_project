import RPi.GPIO as GPIO
import spidev

GPIO.setmode(GPIO.BCM)

GPIO.setup(17, GPIO.OUT)
GPIO.setup(27, GPIO.IN)

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 1000000

def spi_read():
    resp = spi.xfer2([0, 0])
    return resp[0] << 8 | resp[1]

try:
    # GO command to SAM
    GPIO.output(17, 1)
    
    # Waiting for sampling to be done
    while not GPIO.input(27):
        
    
    
finally:
    GPIO.cleanup()
    spi.close()
    