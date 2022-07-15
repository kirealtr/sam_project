import spidev

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 1000000

def spi_read():
    resp = spi.xfer2([0, 0])
    return resp[0] << 8 | resp[1]

try:
    samples = []

    for i in range(13):
        samples.append(spi_read())
        
    print(samples)
finally:
    spi.close()