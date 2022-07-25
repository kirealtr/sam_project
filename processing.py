import numpy as np
import matplotlib.pyplot as plt

ref_voltage = 3.3
period = 0.002 #sampling period in ms
data_size = 4998
length = 1158

data = []
data.append(np.loadtxt('data_0.txt', dtype = int)[:data_size] * (ref_voltage / 4095))
data.append(np.loadtxt('data_1.txt', dtype = int)[:data_size] * (ref_voltage / 4095))

duration = period * data_size
t = np.arange(0, duration, period)

t_0 = np.argmax(data[0]) * period
t_1 = np.argmax(data[1]) * period
sound_speed = length / (t_1 - t_0)

#Make graph
fig, ax = plt.subplots(figsize = (16, 10), dpi = 400)

ax.plot(t, data[0], label = 'V_0(t)', color = 'blue')
ax.plot(t, data[1], label = 'V_1(t)', color = 'red')

ax.set(title = ('Microphones output'), 
       xlabel = ('Time, ms'), ylabel = ('Voltage, V'),
       xlim = (0, duration), ylim = (0, 3.5) )

ax.text(8.2, 3.12, 'Speed of sound: {:.2f} m/s'.format(sound_speed), bbox=dict(facecolor='white', alpha=0.5))

ax.minorticks_on()

ax.grid(which='major', linewidth = 2)
ax.grid(which='minor', linestyle = ':')

ax.legend()

fig.savefig('graph.svg')

print(sound_speed)
