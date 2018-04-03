import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(12,7))

# results from serial version of pi calculation
sgemm_serial = [0.004016, 4.45341, 32.96204]

# results from openmp on host version of sgemm calculation: 8 threads
sgemm_omp_8threads = [0.008632, 0.598037, 2.30442, 21.6578, 86.0919, 185.08]

# results from openmp on host version of sgemm calculation: 16 threads
sgemm_omp_16threads = [0.008541, 0.600694, 2.20316, 21.6075, 86.2309, 180.257]

# results from openmp on host version of sgemm calculation: 32 threads
sgemm_omp_32threads = [0.0086, 0.611338, 3.13636, 21.6355, 86.3669, 186.199]

# results from openmp on host version of sgemm calculation: 48 threads
sgemm_omp_48threads = [0.00758, 0.596559, 2.44682, 22.5264, 87.4749, 185.866]

loops = ('100', '1000', '2000', '4000', '6000', '8000', '10000')
x_pos = np.arange(len(sgemm_serial))

plt.plot(sgemm_serial, label = 'sgemm_serial', color='black', marker='*')

plt.plot(sgemm_omp_8threads, label = 'sgemm_omp_8threads')
plt.plot(sgemm_omp_16threads, label = 'sgemm_omp_16threads', linestyle='--', marker='o')
plt.plot(sgemm_omp_32threads, label = 'sgemm_omp_32threads', linestyle=':', marker='s')
plt.plot(sgemm_omp_48threads, label = 'sgemm_omp_48threads')

plt.xlabel('Matrix sizes (N)')
plt.xticks(x_pos, loops)
plt.ylabel('Execution time (s)')
plt.title('SGEMM Calculation in Parallel Computing')
plt.grid(True)
plt.legend()

plt.show()