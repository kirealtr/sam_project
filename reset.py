import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
GO_pin = 17
is_sampled_pin = 27
ch_select_pin = 22
is_written_pin = 23

GPIO.setup(GO_pin, GPIO.OUT)
GPIO.setup(is_sampled_pin, GPIO.IN)
GPIO.setup(ch_select_pin, GPIO.OUT)
GPIO.setup(is_written_pin, GPIO.IN)

GPIO.output(GO_pin, 0)

GPIO.cleanup()