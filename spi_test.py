import spidev

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1000000

def getAdc():
    adcResponse = spi.xfer2([0])
#    return ((adcResponse[0] & 0x80) >> 7) | ((adcResponse[0] & 0x7F) << 1)
    return adcResponse[0]

try:
    samples = []

    for i in range(13):
        samples.append(getAdc())
    print(samples)
        
finally:
    spi.close()