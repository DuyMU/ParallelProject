import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(12,7))

# results from serial version of pi calculation
pi_serial = [0.032057, 0.189896, 1.88133, 3.59567, 5.39131, 7.20432, 9.18043]

# results from openmp on host version of pi calculation: 8 threads
pi_omp_8threads = [0.017853, 0.1260406, 0.957582, 2.79258, 3.98726, 6.02773, 7.17682]

# results from openmp on host version of pi calculation: 16 threads
pi_omp_16threads = [0.025805, 0.090273, 0.79521, 1.48754, 3.14657, 4.21729, 5.51528]

# results from openmp on host version of pi calculation: 32 threads
pi_omp_32threads = [0.026626, 0.122608, 0.765925, 2.0815, 3.6231, 4.39859, 6.04564]

# results from openmp on host version of pi calculation: 48 threads
pi_omp_48threads = [0.021296, 0.106488, 1.07532, 1.68954, 3.04686, 3.42546, 5.01254]

loops = ('1000000', '10000000', '100M', '200M', '300M', '400M', '500M')
x_pos = np.arange(len(pi_serial))

plt.plot(pi_serial, label = 'pi_serial', color='black', marker='*')

plt.plot(pi_omp_8threads, label = 'pi_omp_8threads')
plt.plot(pi_omp_16threads, label = 'pi_omp_16threads', linestyle='--', marker='o')
plt.plot(pi_omp_32threads, label = 'pi_omp_32threads', linestyle=':', marker='s')
plt.plot(pi_omp_48threads, label = 'pi_omp_48threads')

plt.xlabel('Loop sizes (N)')
plt.xticks(x_pos, loops)
plt.ylabel('Execution time (s)')
plt.title('Pi Calculation in Parallel Computing by Riemann Integral method')
plt.grid(True)
plt.legend()

plt.show()