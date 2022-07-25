import RPi.GPIO as GPIO
import spidev
import matplotlib.pyplot as plt

GPIO.setmode(GPIO.BCM)

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 5000000

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

def write_list(arr, file_name):
    arr_str = list(map(str, arr))
    with open(file_name, 'w') as file:
        file.write('\n'.join(arr_str))

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
        while not GPIO.input(is_written_pin):
            sample = spi_read()
            data[ch].append(sample)
    
    GPIO.output(GO_pin, 0)
    
    data[0].pop(0)
    data[1].pop(0)
    plt.plot(data[0]) #blue
    plt.plot(data[1]) #orange
    
    write_list(data[0], 'data_0.txt')
    write_list(data[1], 'data_1.txt')
    
    plt.show()
    
finally:
    GPIO.output(GO_pin, 0)
    GPIO.cleanup()
    spi.close()
    