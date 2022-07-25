import numpy as np

results = []
results.append(np.loadtxt('clear_air.txt', dtype = float))
results.append(np.loadtxt('after_exhale.txt', dtype = float))

values = [0, 0]
errors = [0, 0]

durations = np.loadtxt('errors.txt', dtype = float)

for i in range(2):
    values[i] = np.mean(results[i])
    
    sist_error = values[i] / durations[i]
    random_error = (np.var(results[i]) / len(results[i]))**0.5
    errors[i] = (sist_error**2 + random_error**2)**0.5
    
with open('result.txt', 'w') as file:
    file.write('Speed of sound in clear air: {:.2f} +- {:.2f}\n'.format(values[0], errors[0]))
    file.write('Speed of sound in air after exhale: {:.2f} +- {:.2f}\n'.format(values[1], errors[1]))
