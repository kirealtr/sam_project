import spidev

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1600000

def getAdc():
    adcResponse = spi.xfer2([0, 0])
    return ((adcResponse[0] & 0x1F) << 8 | adcResponse[1]) >> 1

try:
    samples = []
    
    for i in range(20000):
        samples.append(getAdc())
    print(samples[:10])
        
finally:
    spi.close()