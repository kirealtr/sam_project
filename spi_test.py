import spidev

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 1000000

def getAdc():
    adcResponse = spi.xfer2([0])
    return adcResponse[0]

try:
    samples = []

    for i in range(13):
        print(getAdc())
        
finally:
    spi.close()