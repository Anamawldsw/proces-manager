import sys
import os
import time

# Dodaje cpp_modules do ścieżki importu
sys.path.append(os.path.join(os.path.dirname(__file__), 'cpp_modules'))

import main_cpp



try:
    while True:
        main_cpp.init_all_metrics()

        ram_used = main_cpp.get_total_ram_used_mb()
        cpu_time = main_cpp.get_total_cpu_time()
        io_read = main_cpp.get_total_io_read_mb()
        io_write = main_cpp.get_total_io_write_mb()
        uptime_ms = main_cpp.get_uptime_ms()
        num_cores = main_cpp.get_cpu_count()

        cpu_percent = (cpu_time / (uptime_ms * num_cores)) * 100 if uptime_ms > 0 else 0

        
        print("\033[2J", end="")
        print("\033[H", end="")
        
        print("=" * 25, "SYSTEM TOTAL", "=" * 25, flush=True)
        print(f" Total RAM used      = {ram_used} MB", flush=True)
        print(f" Total CPU time      = {cpu_time} ms ({cpu_percent:.2f}%)", flush=True)
        print(f" Total IO Read       = {io_read} MB", flush=True)
        print(f" Total IO Write      = {io_write} MB", flush=True)


        processes = main_cpp.get_process_list()
        print("=" * 25, "PROCESS LIST", "=" * 25)
        for proc in processes:
            print(f"{proc['pid']:>5} | {proc['name']:<40} | "
                f"RAM: {proc.get('ram_kb', 0):<10} KB | "
                f"CPU: {proc.get('cpu_time_ms', 0):<10} ms | "
                f"IO Read: {proc.get('io_read_kb', 0):<10} KB | "
                f"IO Write: {proc.get('io_write_kb', 0):<10} KB")
        time.sleep(0.5)


except KeyboardInterrupt:
    print("\nMonitoring zakończony.")