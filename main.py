import sys
import os
import time
import threading
import tkinter as tk

sys.path.append(os.path.join(os.path.dirname(__file__), 'cpp_modules'))

import main_cpp


class MonitorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("System Monitor")

        self.text = tk.Text(root, width=160, height=80, font=("Courier", 10))
        self.text.pack()

        self.stop_flag = False

        self.update_loop()

    def update_loop(self):
        if self.stop_flag:
            return

        prev_cpu = main_cpp.get_total_cpu_time()
        prev_uptime = main_cpp.get_uptime_ms()

        time.sleep(1)

        main_cpp.init_all_metrics()

        ram_used = main_cpp.get_total_ram_used_mb()
        curr_cpu = main_cpp.get_total_cpu_time()
        curr_uptime = main_cpp.get_uptime_ms()
        io_read = main_cpp.get_total_io_read_mb()
        io_write = main_cpp.get_total_io_write_mb()
        num_cores = main_cpp.get_cpu_count()

        cpu_delta = curr_cpu - prev_cpu
        time_delta = (curr_uptime - prev_uptime) * num_cores
        cpu_percent = (cpu_delta / time_delta) * 100 if time_delta > 0 else 0

        processes = main_cpp.get_process_list()

        output = []
        output.append("=" * 77 + " SYSTEM TOTAL " + "=" * 50)
        output.append(f" Total RAM used      = {ram_used} MB")
        output.append(f" Total CPU time      = {curr_cpu} ms ({cpu_percent:.2f}%)")
        output.append(f" Total IO Read       = {io_read} MB")
        output.append(f" Total IO Write      = {io_write} MB")
        output.append("")
        output.append("=" * 77 + " PROCESS LIST " + "=" * 50)
        for proc in processes:
            output.append(
                f"{proc['pid']:>5} | {proc['name']:<40} | "
                f"RAM: {proc.get('ram_kb',0):<10} KB | "
                f"CPU: {proc.get('cpu_time_ms',0):<10} ms | "
                f"IO Read: {proc.get('io_read_kb',0):<10} KB | "
                f"IO Write: {proc.get('io_write_kb',0):<10} KB"
            )

        scroll_pos = self.text.yview()

        self.text.delete(1.0, tk.END)
        self.text.insert(tk.END, "\n".join(output))

        self.text.yview_moveto(scroll_pos[0])


        self.root.after(1000, self.update_loop)


def main():
    root = tk.Tk()
    app = MonitorApp(root)
    try:
        root.mainloop()
    except KeyboardInterrupt:
        print("\nMonitoring zakończony.")


if __name__ == "__main__":
    main()
