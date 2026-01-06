/*
 * RayuOS Main Command Utility (rayu)
 * 
 * A unified command for RayuOS that provides:
 * - System monitoring and information
 * - Package management (apt wrapper)
 * - Architecture and OS information
 * 
 * Usage:
 *   rayu              - Show system status/monitoring
 *   rayu info         - Show detailed system info
 *   rayu install PKG  - Install package (wraps apt)
 *   rayu remove PKG   - Remove package (wraps apt)
 *   rayu update       - Update package lists (wraps apt)
 *   rayu upgrade      - Upgrade packages (wraps apt)
 *   rayu search PKG   - Search for packages (wraps apt)
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
#include <dirent.h>
#include <ctype.h>

#define VERSION "1.0.0"
#define RAYUOS_VERSION "1.0"

/* ANSI color codes */
#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_WHITE   "\033[37m"

/* Function prototypes */
static void print_logo(void);
static void print_usage(void);
static void show_status(void);
static void show_info(void);
static void show_monitor(void);
static int run_apt_command(int argc, char *argv[]);
static char *human_size(unsigned long bytes, char *buf, size_t len);
static char *human_time(long secs, char *buf, size_t len);
static int get_cpu_usage(void);
static int count_processes(void);
static int count_packages(void);

/* Print RayuOS logo */
static void print_logo(void)
{
    printf(C_CYAN C_BOLD);
    printf("  ____                   ___  ____  \n");
    printf(" |  _ \\ __ _ _   _ _   _/ _ \\/ ___| \n");
    printf(" | |_) / _` | | | | | | | | | \\___ \\ \n");
    printf(" |  _ < (_| | |_| | |_| | |_| |___) |\n");
    printf(" |_| \\_\\__,_|\\__, |\\__,_|\\___/|____/ \n");
    printf("             |___/   v%s\n", RAYUOS_VERSION);
    printf(C_RESET "\n");
}

/* Print usage information */
static void print_usage(void)
{
    printf("Usage: rayu [COMMAND] [OPTIONS]\n\n");
    printf(C_BOLD "System Commands:\n" C_RESET);
    printf("  (no command)    Show system status overview\n");
    printf("  info            Show detailed system information\n");
    printf("  monitor         Live system monitoring (Ctrl+C to exit)\n");
    printf("  version         Show RayuOS version\n\n");
    printf(C_BOLD "Package Management (apt wrapper):\n" C_RESET);
    printf("  install <pkg>   Install a package\n");
    printf("  remove <pkg>    Remove a package\n");
    printf("  update          Update package lists\n");
    printf("  upgrade         Upgrade all packages\n");
    printf("  search <pkg>    Search for packages\n");
    printf("  list            List installed packages\n");
    printf("  clean           Clean package cache\n\n");
    printf(C_BOLD "Examples:\n" C_RESET);
    printf("  rayu                    # Show system status\n");
    printf("  rayu install htop       # Install htop\n");
    printf("  rayu update && rayu upgrade\n");
    printf("\n");
}

/* Convert bytes to human readable */
static char *human_size(unsigned long bytes, char *buf, size_t len)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int u = 0;
    double val = bytes;
    while (val >= 1024 && u < 4) { val /= 1024; u++; }
    snprintf(buf, len, "%.1f %s", val, units[u]);
    return buf;
}

/* Convert seconds to human readable time */
static char *human_time(long secs, char *buf, size_t len)
{
    int d = secs / 86400;
    int h = (secs % 86400) / 3600;
    int m = (secs % 3600) / 60;
    if (d > 0) snprintf(buf, len, "%dd %dh %dm", d, h, m);
    else if (h > 0) snprintf(buf, len, "%dh %dm", h, m);
    else snprintf(buf, len, "%dm", m);
    return buf;
}

/* Get approximate CPU usage (simple method) */
static int get_cpu_usage(void)
{
    static long prev_idle = 0, prev_total = 0;
    long idle = 0, total = 0;
    long user, nice, system, idle_t, iowait, irq, softirq;
    
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;
    
    if (fscanf(f, "cpu %ld %ld %ld %ld %ld %ld %ld",
               &user, &nice, &system, &idle_t, &iowait, &irq, &softirq) == 7) {
        idle = idle_t + iowait;
        total = user + nice + system + idle_t + iowait + irq + softirq;
    }
    fclose(f);
    
    int usage = 0;
    if (prev_total > 0) {
        long diff_idle = idle - prev_idle;
        long diff_total = total - prev_total;
        if (diff_total > 0) {
            usage = (int)(100 * (diff_total - diff_idle) / diff_total);
        }
    }
    
    prev_idle = idle;
    prev_total = total;
    return usage;
}

/* Count running processes */
static int count_processes(void)
{
    int count = 0;
    DIR *dir = opendir("/proc");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (isdigit(entry->d_name[0])) count++;
        }
        closedir(dir);
    }
    return count;
}

/* Count installed packages */
static int count_packages(void)
{
    int count = 0;
    FILE *f = popen("dpkg -l 2>/dev/null | grep -c '^ii'", "r");
    if (f) {
        fscanf(f, "%d", &count);
        pclose(f);
    }
    return count;
}

/* Progress bar helper */
static void print_bar(int percent, int width, const char *color)
{
    int filled = (percent * width) / 100;
    printf("%s[", color);
    for (int i = 0; i < width; i++) {
        printf(i < filled ? "█" : "░");
    }
    printf("]" C_RESET " %3d%%", percent);
}

/* Show system status (default command) */
static void show_status(void)
{
    struct utsname uts;
    struct sysinfo si;
    char buf1[32], buf2[32], buf3[32];
    
    print_logo();
    
    /* Get system info */
    uname(&uts);
    sysinfo(&si);
    
    /* OS and Kernel */
    char os_name[64] = "RayuOS";
    FILE *f = fopen("/etc/os-release", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *start = strchr(line, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end) { *end = '\0'; strncpy(os_name, start, sizeof(os_name)-1); }
                }
                break;
            }
        }
        fclose(f);
    }
    
    printf(C_CYAN "  OS:        " C_RESET "%s\n", os_name);
    printf(C_CYAN "  Kernel:    " C_RESET "%s %s\n", uts.sysname, uts.release);
    printf(C_CYAN "  Arch:      " C_RESET "%s\n", uts.machine);
    printf(C_CYAN "  Host:      " C_RESET "%s\n", uts.nodename);
    printf(C_CYAN "  Uptime:    " C_RESET "%s\n", human_time(si.uptime, buf1, sizeof(buf1)));
    printf(C_CYAN "  Packages:  " C_RESET "%d (dpkg)\n", count_packages());
    printf(C_CYAN "  Processes: " C_RESET "%d\n", count_processes());
    printf("\n");
    
    /* Memory */
    unsigned long mem_total = si.totalram * si.mem_unit;
    unsigned long mem_used = (si.totalram - si.freeram - si.bufferram) * si.mem_unit;
    int mem_pct = (int)((mem_used * 100) / mem_total);
    
    printf(C_CYAN "  Memory:    " C_RESET);
    print_bar(mem_pct, 20, mem_pct > 80 ? C_RED : (mem_pct > 60 ? C_YELLOW : C_GREEN));
    printf("  %s / %s\n", human_size(mem_used, buf1, 32), human_size(mem_total, buf2, 32));
    
    /* Swap */
    if (si.totalswap > 0) {
        unsigned long swap_total = si.totalswap * si.mem_unit;
        unsigned long swap_used = (si.totalswap - si.freeswap) * si.mem_unit;
        int swap_pct = (int)((swap_used * 100) / swap_total);
        
        printf(C_CYAN "  Swap:      " C_RESET);
        print_bar(swap_pct, 20, swap_pct > 80 ? C_RED : C_GREEN);
        printf("  %s / %s\n", human_size(swap_used, buf1, 32), human_size(swap_total, buf2, 32));
    }
    
    /* Disk */
    struct statvfs st;
    if (statvfs("/", &st) == 0) {
        unsigned long disk_total = st.f_blocks * st.f_frsize;
        unsigned long disk_used = (st.f_blocks - st.f_bfree) * st.f_frsize;
        int disk_pct = (int)((disk_used * 100) / disk_total);
        
        printf(C_CYAN "  Disk (/):  " C_RESET);
        print_bar(disk_pct, 20, disk_pct > 90 ? C_RED : (disk_pct > 70 ? C_YELLOW : C_GREEN));
        printf("  %s / %s\n", human_size(disk_used, buf1, 32), human_size(disk_total, buf2, 32));
    }
    
    /* Load average */
    printf(C_CYAN "  Load:      " C_RESET "%.2f, %.2f, %.2f\n",
           si.loads[0]/65536.0, si.loads[1]/65536.0, si.loads[2]/65536.0);
    
    printf("\n");
    printf(C_BOLD "  Tip:" C_RESET " Use 'rayu --help' for all commands\n\n");
}

/* Show detailed system info */
static void show_info(void)
{
    struct utsname uts;
    struct sysinfo si;
    char buf[64];
    
    print_logo();
    uname(&uts);
    sysinfo(&si);
    
    printf(C_BOLD C_CYAN "═══ System Information ═══\n\n" C_RESET);
    
    /* OS Info */
    printf(C_BOLD "Operating System:\n" C_RESET);
    FILE *f = fopen("/etc/os-release", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0 ||
                strncmp(line, "VERSION=", 8) == 0 ||
                strncmp(line, "ID=", 3) == 0) {
                char *eq = strchr(line, '=');
                if (eq) {
                    *eq = '\0';
                    char *val = eq + 1;
                    /* Remove quotes and newline */
                    if (*val == '"') val++;
                    char *end = strpbrk(val, "\"\n");
                    if (end) *end = '\0';
                    printf("  %-14s %s\n", line, val);
                }
            }
        }
        fclose(f);
    }
    printf("\n");
    
    /* Kernel Info */
    printf(C_BOLD "Kernel:\n" C_RESET);
    printf("  %-14s %s\n", "System", uts.sysname);
    printf("  %-14s %s\n", "Release", uts.release);
    printf("  %-14s %s\n", "Version", uts.version);
    printf("  %-14s %s\n", "Machine", uts.machine);
    printf("  %-14s %s\n", "Hostname", uts.nodename);
    printf("\n");
    
    /* CPU Info */
    printf(C_BOLD "Processor:\n" C_RESET);
    f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        int shown_model = 0, cores = 0;
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "model name", 10) == 0 && !shown_model) {
                char *colon = strchr(line, ':');
                if (colon) {
                    colon++; while (*colon == ' ') colon++;
                    char *nl = strchr(colon, '\n'); if (nl) *nl = '\0';
                    printf("  %-14s %s\n", "Model", colon);
                    shown_model = 1;
                }
            }
            if (strncmp(line, "processor", 9) == 0) cores++;
        }
        printf("  %-14s %d\n", "Cores", cores);
        fclose(f);
    }
    printf("\n");
    
    /* Memory Info */
    printf(C_BOLD "Memory:\n" C_RESET);
    unsigned long mem_total = si.totalram * si.mem_unit;
    unsigned long mem_free = si.freeram * si.mem_unit;
    unsigned long mem_buffers = si.bufferram * si.mem_unit;
    printf("  %-14s %s\n", "Total", human_size(mem_total, buf, sizeof(buf)));
    printf("  %-14s %s\n", "Free", human_size(mem_free, buf, sizeof(buf)));
    printf("  %-14s %s\n", "Buffers", human_size(mem_buffers, buf, sizeof(buf)));
    if (si.totalswap > 0) {
        printf("  %-14s %s\n", "Swap Total", human_size(si.totalswap * si.mem_unit, buf, sizeof(buf)));
        printf("  %-14s %s\n", "Swap Free", human_size(si.freeswap * si.mem_unit, buf, sizeof(buf)));
    }
    printf("\n");
    
    /* Network */
    printf(C_BOLD "Network Interfaces:\n" C_RESET);
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == 0) {
        for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr || strcmp(ifa->ifa_name, "lo") == 0) continue;
            if (ifa->ifa_addr->sa_family == AF_INET) {
                char ip[INET_ADDRSTRLEN];
                struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
                inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
                printf("  %-14s %s\n", ifa->ifa_name, ip);
            }
        }
        freeifaddrs(ifaddr);
    }
    printf("\n");
}

/* Live monitoring mode */
static void show_monitor(void)
{
    struct sysinfo si;
    char buf1[32], buf2[32];
    
    printf("\033[2J\033[H"); /* Clear screen */
    printf(C_BOLD C_CYAN "RayuOS System Monitor" C_RESET " (Press Ctrl+C to exit)\n\n");
    
    /* Prime CPU usage calculation */
    get_cpu_usage();
    usleep(100000);
    
    while (1) {
        printf("\033[3;0H"); /* Move cursor to line 3 */
        
        sysinfo(&si);
        
        /* CPU */
        int cpu = get_cpu_usage();
        printf(C_CYAN "CPU:    " C_RESET);
        print_bar(cpu, 30, cpu > 80 ? C_RED : (cpu > 50 ? C_YELLOW : C_GREEN));
        printf("          \n");
        
        /* Memory */
        unsigned long mem_total = si.totalram * si.mem_unit;
        unsigned long mem_used = (si.totalram - si.freeram - si.bufferram) * si.mem_unit;
        int mem_pct = (int)((mem_used * 100) / mem_total);
        
        printf(C_CYAN "Memory: " C_RESET);
        print_bar(mem_pct, 30, mem_pct > 80 ? C_RED : (mem_pct > 60 ? C_YELLOW : C_GREEN));
        printf(" %s / %s    \n", human_size(mem_used, buf1, 32), human_size(mem_total, buf2, 32));
        
        /* Swap */
        if (si.totalswap > 0) {
            unsigned long swap_total = si.totalswap * si.mem_unit;
            unsigned long swap_used = (si.totalswap - si.freeswap) * si.mem_unit;
            int swap_pct = (int)((swap_used * 100) / swap_total);
            
            printf(C_CYAN "Swap:   " C_RESET);
            print_bar(swap_pct, 30, swap_pct > 50 ? C_YELLOW : C_GREEN);
            printf(" %s / %s    \n", human_size(swap_used, buf1, 32), human_size(swap_total, buf2, 32));
        }
        
        /* Load and processes */
        printf("\n");
        printf(C_CYAN "Load:   " C_RESET "%.2f %.2f %.2f    \n",
               si.loads[0]/65536.0, si.loads[1]/65536.0, si.loads[2]/65536.0);
        printf(C_CYAN "Procs:  " C_RESET "%d    \n", count_processes());
        printf(C_CYAN "Uptime: " C_RESET "%s    \n", human_time(si.uptime, buf1, sizeof(buf1)));
        
        fflush(stdout);
        sleep(1);
    }
}

/* Run apt command (wrapper) */
static int run_apt_command(int argc, char *argv[])
{
    char cmd[1024] = "apt ";
    
    /* Map rayu commands to apt commands */
    const char *rayu_cmd = argv[1];
    
    if (strcmp(rayu_cmd, "install") == 0 ||
        strcmp(rayu_cmd, "remove") == 0 ||
        strcmp(rayu_cmd, "purge") == 0 ||
        strcmp(rayu_cmd, "update") == 0 ||
        strcmp(rayu_cmd, "upgrade") == 0 ||
        strcmp(rayu_cmd, "search") == 0 ||
        strcmp(rayu_cmd, "show") == 0 ||
        strcmp(rayu_cmd, "list") == 0 ||
        strcmp(rayu_cmd, "autoremove") == 0 ||
        strcmp(rayu_cmd, "clean") == 0 ||
        strcmp(rayu_cmd, "autoclean") == 0) {
        
        /* Build the apt command */
        for (int i = 1; i < argc; i++) {
            strcat(cmd, argv[i]);
            strcat(cmd, " ");
        }
        
        /* Print what we're doing */
        printf(C_CYAN C_BOLD "rayu:" C_RESET " Running 'apt %s'\n\n", rayu_cmd);
        
        return system(cmd);
    }
    
    /* Unknown command */
    fprintf(stderr, C_RED "Unknown command: %s\n" C_RESET, rayu_cmd);
    fprintf(stderr, "Use 'rayu --help' for available commands.\n");
    return 1;
}

/* Main entry point */
int main(int argc, char *argv[])
{
    /* No arguments - show status */
    if (argc == 1) {
        show_status();
        return 0;
    }
    
    const char *cmd = argv[1];
    
    /* Help */
    if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0 || strcmp(cmd, "help") == 0) {
        print_logo();
        print_usage();
        return 0;
    }
    
    /* Version */
    if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0 || strcmp(cmd, "version") == 0) {
        printf("rayu (RayuOS) version %s\n", VERSION);
        printf("RayuOS version %s\n", RAYUOS_VERSION);
        return 0;
    }
    
    /* Info */
    if (strcmp(cmd, "info") == 0) {
        show_info();
        return 0;
    }
    
    /* Monitor */
    if (strcmp(cmd, "monitor") == 0 || strcmp(cmd, "mon") == 0) {
        show_monitor();
        return 0;
    }
    
    /* Status (explicit) */
    if (strcmp(cmd, "status") == 0) {
        show_status();
        return 0;
    }
    
    /* Package management - pass to apt */
    return run_apt_command(argc, argv);
}
