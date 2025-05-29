import sys
import os

# Dodaje cpp_modules do ścieżki importu
sys.path.append(os.path.join(os.path.dirname(__file__), 'cpp_modules'))

import main_cpp

main_cpp.init_all_metrics()

ram_used = main_cpp.get_total_ram_used_mb()
cpu_time = main_cpp.get_total_cpu_time()
io_read = main_cpp.get_total_io_read_mb()
io_write = main_cpp.get_total_io_write_mb()
uptime_ms = main_cpp.get_uptime_ms()
num_cores = main_cpp.get_cpu_count()

cpu_percent = (cpu_time / (uptime_ms * num_cores)) * 100 if uptime_ms > 0 else 0


print("=" * 25, "SYSTEM TOTAL", "=" * 25)
print(f" Total RAM used      = {ram_used} MB")
print(f" Total CPU time      = {cpu_time} ms ({cpu_percent:.2f}%)")
print(f" Total IO Read       = {io_read} MB")
print(f" Total IO Write      = {io_write} MB")