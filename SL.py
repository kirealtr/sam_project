import RPi.GPIO as GPIO
import spidev
import matplotlib.pyplot as plt

GPIO.setmode(GPIO.BCM)

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 10000000

GO_pin = 17
is_sampled_pin = 27
ch_select_pin = 22
is_written_pin = 23

GPIO.setup(GO_pin, GPIO.OUT)
GPIO.setup(is_sampled_pin, GPIO.IN)
GPIO.setup(ch_select_pin, GPIO.OUT)
GPIO.setup(is_written_pin, GPIO.IN)

def spi_read():
    resp = spi.xfer2([0, 0])
    return resp[0] << 8 | resp[1]

GPIO.output(GO_pin, 0)

try:
    data = [[] for _ in range(2)]
    # GO command to SAM
    GPIO.output(GO_pin, 1)
    
    # Waiting for sampling to be done
    while not GPIO.input(is_sampled_pin):
        continue
        
    # Reading from channels
    for ch in range(2):
        GPIO.output(ch_select_pin, ch)
        
        while GPIO.input(is_written_pin):
            continue
        #print(ch)
        while not GPIO.input(is_written_pin):
            data[ch].append(spi_read())
            print(GPIO.input(is_written_pin))
        print(ch)
    
    GPIO.output(GO_pin, 0)
    plt.plot(data[0])
    plt.plot(data[1])
    
    plt.show()
    
finally:
    GPIO.cleanup()
    spi.close()
    