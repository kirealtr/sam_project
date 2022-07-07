import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

GPIO.setup(17, GPIO.OUT)
GPIO.setup(27, GPIO.IN)

try:
    # GO command to SAM
    GPIO.output(17, 1)
    
    # Waiting for sampling to be done
    while not GPIO.input(27):
        print('...')
        
    print('Got it!')
    
finally:
    GPIO.cleanup()
