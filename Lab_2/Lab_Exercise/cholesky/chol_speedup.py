import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(12,7))

# results from serial version of pi calculation
chol_serial = [0.000721, 0.634465, 5.00225, 40.096, 136.431, 323.151, ]

# results from openmp on host version of chol calculation: 8 threads
chol_omp_8threads = [0.005136, 0.192166, 0.827317, 3.66344, 9.24924, 19.4674, 37.5319]

# results from openmp on host version of chol calculation: 16 threads
chol_omp_16threads = [0.005997, 0.194117, 0.831226, 3.21844, 9.29916, 20.5382, 37.0182]

# results from openmp on host version of chol calculation: 32 threads
chol_omp_32threads = [0.006361, 0.192553, 0.785325, 3.29344, 9.11284, 20.1299, 38.5062]

# results from openmp on host version of chol calculation: 48 threads
chol_omp_48threads = [0.005295, 0.195904, 0.78963, 3.67192, 9.55611, 20.9759, 38.1312]

loops = ('100', '1000', '2000', '4000', '6000', '8000', '10000')
x_pos = np.arange(len(chol_serial))

plt.plot(chol_serial, label = 'chol_serial', color='black', marker='*')

plt.plot(chol_omp_8threads, label = 'chol_omp_8threads')
plt.plot(chol_omp_16threads, label = 'chol_omp_16threads', linestyle='--', marker='o')
plt.plot(chol_omp_32threads, label = 'chol_omp_32threads', linestyle=':', marker='s')
plt.plot(chol_omp_48threads, label = 'chol_omp_48threads')

plt.xlabel('Matrix sizes (N)')
plt.xticks(x_pos, loops)
plt.ylabel('Execution time (s)')
plt.title('chol Calculation in Parallel Computing')
plt.grid(True)
plt.legend()

plt.show()