/*
 * RayuOS System Information Utility
 * 
 * A lightweight C program to display system information.
 * Shows: OS info, kernel, CPU, memory, disk, network, uptime.
 * 
 * Copyright (c) 2026 RayuOS Project
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define VERSION "1.0.0"

/* ANSI color codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

/* Function prototypes */
static void print_banner(void);
static void print_os_info(void);
static void print_kernel_info(void);
static void print_cpu_info(void);
static void print_memory_info(void);
static void print_disk_info(void);
static void print_network_info(void);
static void print_uptime(void);
static char *human_readable_size(unsigned long bytes, char *buf, size_t size);
static char *human_readable_time(long seconds, char *buf, size_t size);
static int read_file_line(const char *path, char *buffer, size_t size);

/* Print the RayuOS banner */
static void print_banner(void)
{
    printf(COLOR_CYAN);
    printf("  ____                   ___  ____  \n");
    printf(" |  _ \\ __ _ _   _ _   _/ _ \\/ ___| \n");
    printf(" | |_) / _` | | | | | | | | | \\___ \\ \n");
    printf(" |  _ < (_| | |_| | |_| | |_| |___) |\n");
    printf(" |_| \\_\\__,_|\\__, |\\__,_|\\___/|____/ \n");
    printf("             |___/                   \n");
    printf(COLOR_RESET "\n");
}

/* Print OS information from /etc/os-release */
static void print_os_info(void)
{
    char line[256];
    char pretty_name[256] = "Unknown";
    
    FILE *f = fopen("/etc/os-release", "r");
    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                /* Extract value between quotes */
                char *start = strchr(line, '"');
                if (start != NULL) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end != NULL) {
                        *end = '\0';
                        strncpy(pretty_name, start, sizeof(pretty_name) - 1);
                    }
                }
                break;
            }
        }
        fclose(f);
    }
    
    printf(COLOR_CYAN COLOR_BOLD "OS:       " COLOR_RESET "%s\n", pretty_name);
}

/* Print kernel information */
static void print_kernel_info(void)
{
    struct utsname buf;
    
    if (uname(&buf) == 0) {
        printf(COLOR_CYAN COLOR_BOLD "Kernel:   " COLOR_RESET "%s %s\n", buf.sysname, buf.release);
        printf(COLOR_CYAN COLOR_BOLD "Arch:     " COLOR_RESET "%s\n", buf.machine);
        printf(COLOR_CYAN COLOR_BOLD "Hostname: " COLOR_RESET "%s\n", buf.nodename);
    }
}

/* Print CPU information from /proc/cpuinfo */
static void print_cpu_info(void)
{
    char line[256];
    char model_name[256] = "Unknown";
    int cpu_count = 0;
    
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f != NULL) {
        while (fgets(line, sizeof(line), f) != NULL) {
            if (strncmp(line, "model name", 10) == 0) {
                cpu_count++;
                if (cpu_count == 1) {
                    char *colon = strchr(line, ':');
                    if (colon != NULL) {
                        colon++;
                        while (*colon == ' ' || *colon == '\t') colon++;
                        /* Remove trailing newline */
                        char *newline = strchr(colon, '\n');
                        if (newline != NULL) *newline = '\0';
                        strncpy(model_name, colon, sizeof(model_name) - 1);
                    }
                }
            }
        }
        fclose(f);
    }
    
    if (cpu_count == 0) cpu_count = 1;
    
    printf(COLOR_CYAN COLOR_BOLD "CPU:      " COLOR_RESET "%s\n", model_name);
    printf(COLOR_CYAN COLOR_BOLD "Cores:    " COLOR_RESET "%d\n", cpu_count);
}

/* Print memory information from /proc/meminfo */
static void print_memory_info(void)
{
    struct sysinfo info;
    char total_buf[32], used_buf[32], free_buf[32];
    
    if (sysinfo(&info) == 0) {
        unsigned long total = info.totalram * info.mem_unit;
        unsigned long free_mem = info.freeram * info.mem_unit;
        unsigned long buffers = info.bufferram * info.mem_unit;
        unsigned long used = total - free_mem - buffers;
        
        human_readable_size(total, total_buf, sizeof(total_buf));
        human_readable_size(used, used_buf, sizeof(used_buf));
        human_readable_size(free_mem + buffers, free_buf, sizeof(free_buf));
        
        int percent = (int)((used * 100) / total);
        
        /* Color based on usage */
        const char *color = COLOR_GREEN;
        if (percent > 80) color = COLOR_RED;
        else if (percent > 60) color = COLOR_YELLOW;
        
        printf(COLOR_CYAN COLOR_BOLD "Memory:   " COLOR_RESET);
        printf("%s%s" COLOR_RESET " / %s (%d%% used)\n", color, used_buf, total_buf, percent);
        
        /* Swap info */
        if (info.totalswap > 0) {
            unsigned long swap_total = info.totalswap * info.mem_unit;
            unsigned long swap_free = info.freeswap * info.mem_unit;
            unsigned long swap_used = swap_total - swap_free;
            
            human_readable_size(swap_used, used_buf, sizeof(used_buf));
            human_readable_size(swap_total, total_buf, sizeof(total_buf));
            
            printf(COLOR_CYAN COLOR_BOLD "Swap:     " COLOR_RESET "%s / %s\n", used_buf, total_buf);
        }
    }
}

/* Print disk information for root filesystem */
static void print_disk_info(void)
{
    struct statvfs stat;
    char total_buf[32], used_buf[32], avail_buf[32];
    
    if (statvfs("/", &stat) == 0) {
        unsigned long total = stat.f_blocks * stat.f_frsize;
        unsigned long free_space = stat.f_bfree * stat.f_frsize;
        unsigned long avail = stat.f_bavail * stat.f_frsize;
        unsigned long used = total - free_space;
        
        human_readable_size(total, total_buf, sizeof(total_buf));
        human_readable_size(used, used_buf, sizeof(used_buf));
        human_readable_size(avail, avail_buf, sizeof(avail_buf));
        
        int percent = (total > 0) ? (int)((used * 100) / total) : 0;
        
        /* Color based on usage */
        const char *color = COLOR_GREEN;
        if (percent > 90) color = COLOR_RED;
        else if (percent > 70) color = COLOR_YELLOW;
        
        printf(COLOR_CYAN COLOR_BOLD "Disk (/): " COLOR_RESET);
        printf("%s%s" COLOR_RESET " / %s (%d%% used)\n", color, used_buf, total_buf, percent);
    }
}

/* Print network interface information */
static void print_network_info(void)
{
    struct ifaddrs *ifaddr, *ifa;
    int shown = 0;
    
    if (getifaddrs(&ifaddr) == -1) {
        return;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        /* Skip loopback */
        if (strcmp(ifa->ifa_name, "lo") == 0) continue;
        
        /* Only show IPv4 addresses */
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            
            if (shown == 0) {
                printf(COLOR_CYAN COLOR_BOLD "Network:  " COLOR_RESET);
            } else {
                printf("          ");
            }
            printf("%s: %s\n", ifa->ifa_name, ip);
            shown++;
        }
    }
    
    freeifaddrs(ifaddr);
    
    if (shown == 0) {
        printf(COLOR_CYAN COLOR_BOLD "Network:  " COLOR_RESET "No active interfaces\n");
    }
}

/* Print system uptime */
static void print_uptime(void)
{
    struct sysinfo info;
    char uptime_buf[64];
    
    if (sysinfo(&info) == 0) {
        human_readable_time(info.uptime, uptime_buf, sizeof(uptime_buf));
        printf(COLOR_CYAN COLOR_BOLD "Uptime:   " COLOR_RESET "%s\n", uptime_buf);
        
        /* Load average */
        printf(COLOR_CYAN COLOR_BOLD "Load:     " COLOR_RESET "%.2f, %.2f, %.2f\n",
               info.loads[0] / 65536.0,
               info.loads[1] / 65536.0,
               info.loads[2] / 65536.0);
    }
}

/* Convert bytes to human readable format */
static char *human_readable_size(unsigned long bytes, char *buf, size_t size)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double value = bytes;
    
    while (value >= 1024 && unit < 4) {
        value /= 1024;
        unit++;
    }
    
    snprintf(buf, size, "%.1f %s", value, units[unit]);
    return buf;
}

/* Convert seconds to human readable time */
static char *human_readable_time(long seconds, char *buf, size_t size)
{
    int days = seconds / 86400;
    int hours = (seconds % 86400) / 3600;
    int minutes = (seconds % 3600) / 60;
    
    if (days > 0) {
        snprintf(buf, size, "%d days, %d hours, %d minutes", days, hours, minutes);
    } else if (hours > 0) {
        snprintf(buf, size, "%d hours, %d minutes", hours, minutes);
    } else {
        snprintf(buf, size, "%d minutes", minutes);
    }
    
    return buf;
}

/* Read first line from a file */
static int read_file_line(const char *path, char *buffer, size_t size)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return -1;
    }
    
    if (fgets(buffer, size, f) == NULL) {
        fclose(f);
        return -1;
    }
    
    fclose(f);
    
    /* Remove trailing newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return 0;
}

/* Print usage information */
static void print_usage(const char *progname)
{
    printf("Usage: %s [OPTIONS]\n", progname);
    printf("\n");
    printf("Display system information for RayuOS.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("  -n, --no-logo  Don't show the RayuOS logo\n");
    printf("\n");
}

/* Main entry point */
int main(int argc, char *argv[])
{
    int show_logo = 1;
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("rayuos-sysinfo %s\n", VERSION);
            return 0;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-logo") == 0) {
            show_logo = 0;
        }
    }
    
    /* Print banner */
    if (show_logo) {
        print_banner();
    }
    
    /* Print system information */
    print_os_info();
    print_kernel_info();
    print_cpu_info();
    print_memory_info();
    print_disk_info();
    print_network_info();
    print_uptime();
    
    printf("\n");
    
    return 0;
}
