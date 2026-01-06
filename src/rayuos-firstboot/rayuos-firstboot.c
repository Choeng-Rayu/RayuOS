/*
 * RayuOS First Boot Setup Utility
 * 
 * A lightweight C program for initial system configuration.
 * Handles: hostname, username, password, timezone, locale setup.
 * 
 * Copyright (c) 2026 RayuOS Project
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#define VERSION "1.0.0"
#define MAX_INPUT 256
#define MAX_USERNAME 32
#define MAX_HOSTNAME 64

/* ANSI color codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"

/* Function prototypes */
static void print_banner(void);
static void print_step(int step, int total, const char *desc);
static int read_line(const char *prompt, char *buffer, size_t size);
static int read_password(const char *prompt, char *buffer, size_t size);
static int validate_username(const char *username);
static int validate_hostname(const char *hostname);
static int set_hostname(const char *hostname);
static int create_user(const char *username, const char *password);
static int set_timezone(const char *timezone);
static int run_command(const char *cmd);
static int file_exists(const char *path);
static int is_root(void);

/* Print the RayuOS banner */
static void print_banner(void)
{
    printf("\n");
    printf(COLOR_CYAN COLOR_BOLD);
    printf("  ____                   ___  ____  \n");
    printf(" |  _ \\ __ _ _   _ _   _/ _ \\/ ___| \n");
    printf(" | |_) / _` | | | | | | | | | \\___ \\ \n");
    printf(" |  _ < (_| | |_| | |_| | |_| |___) |\n");
    printf(" |_| \\_\\__,_|\\__, |\\__,_|\\___/|____/ \n");
    printf("             |___/                   \n");
    printf(COLOR_RESET "\n");
    printf("  First Boot Setup v%s\n", VERSION);
    printf("  ─────────────────────────────────\n\n");
}

/* Print a step indicator */
static void print_step(int step, int total, const char *desc)
{
    printf(COLOR_GREEN "[%d/%d]" COLOR_RESET " %s\n", step, total, desc);
}

/* Read a line of input with prompt */
static int read_line(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);
    
    if (fgets(buffer, size, stdin) == NULL) {
        return -1;
    }
    
    /* Remove trailing newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return 0;
}

/* Read password without echo */
static int read_password(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);
    
    /* Disable echo */
    int result = run_command("stty -echo");
    if (result != 0) {
        /* Fallback to regular input if stty fails */
        return read_line("", buffer, size);
    }
    
    int ret = read_line("", buffer, size);
    
    /* Re-enable echo */
    run_command("stty echo");
    printf("\n");
    
    return ret;
}

/* Validate username (lowercase, starts with letter, alphanumeric) */
static int validate_username(const char *username)
{
    if (username == NULL || strlen(username) == 0) {
        return 0;
    }
    
    size_t len = strlen(username);
    if (len > MAX_USERNAME) {
        return 0;
    }
    
    /* Must start with lowercase letter */
    if (!islower(username[0])) {
        return 0;
    }
    
    /* Only lowercase letters, digits, underscore, hyphen */
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!islower(c) && !isdigit(c) && c != '_' && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

/* Validate hostname */
static int validate_hostname(const char *hostname)
{
    if (hostname == NULL || strlen(hostname) == 0) {
        return 0;
    }
    
    size_t len = strlen(hostname);
    if (len > MAX_HOSTNAME) {
        return 0;
    }
    
    /* Only alphanumeric and hyphens, no leading/trailing hyphen */
    if (hostname[0] == '-' || hostname[len - 1] == '-') {
        return 0;
    }
    
    for (size_t i = 0; i < len; i++) {
        char c = hostname[i];
        if (!isalnum(c) && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

/* Set the system hostname */
static int set_hostname(const char *hostname)
{
    char cmd[512];
    
    /* Write to /etc/hostname */
    FILE *f = fopen("/etc/hostname", "w");
    if (f == NULL) {
        perror("Failed to open /etc/hostname");
        return -1;
    }
    fprintf(f, "%s\n", hostname);
    fclose(f);
    
    /* Update /etc/hosts */
    snprintf(cmd, sizeof(cmd), 
             "sed -i 's/127.0.1.1.*/127.0.1.1\\t%s/' /etc/hosts", hostname);
    run_command(cmd);
    
    /* Set hostname immediately */
    snprintf(cmd, sizeof(cmd), "hostnamectl set-hostname %s 2>/dev/null || hostname %s", 
             hostname, hostname);
    return run_command(cmd);
}

/* Create a new user with password */
static int create_user(const char *username, const char *password)
{
    char cmd[512];
    
    /* Create user with home directory and add to common groups */
    snprintf(cmd, sizeof(cmd),
             "useradd -m -s /bin/bash -G sudo,audio,video,netdev,plugdev %s 2>/dev/null",
             username);
    
    int result = run_command(cmd);
    if (result != 0) {
        /* User might already exist, try to continue */
        printf(COLOR_YELLOW "Note: User may already exist, updating password.\n" COLOR_RESET);
    }
    
    /* Set password using chpasswd */
    snprintf(cmd, sizeof(cmd), "echo '%s:%s' | chpasswd", username, password);
    result = run_command(cmd);
    
    if (result != 0) {
        fprintf(stderr, "Failed to set password for user %s\n", username);
        return -1;
    }
    
    return 0;
}

/* Set timezone */
static int set_timezone(const char *timezone)
{
    char cmd[512];
    char tzpath[256];
    
    /* Verify timezone file exists */
    snprintf(tzpath, sizeof(tzpath), "/usr/share/zoneinfo/%s", timezone);
    if (!file_exists(tzpath)) {
        fprintf(stderr, "Invalid timezone: %s\n", timezone);
        return -1;
    }
    
    /* Set timezone using timedatectl or manually */
    snprintf(cmd, sizeof(cmd), "timedatectl set-timezone %s 2>/dev/null", timezone);
    if (run_command(cmd) != 0) {
        /* Fallback: manually link timezone file */
        snprintf(cmd, sizeof(cmd), "ln -sf /usr/share/zoneinfo/%s /etc/localtime", timezone);
        run_command(cmd);
        
        /* Write timezone name */
        FILE *f = fopen("/etc/timezone", "w");
        if (f != NULL) {
            fprintf(f, "%s\n", timezone);
            fclose(f);
        }
    }
    
    return 0;
}

/* Run a shell command */
static int run_command(const char *cmd)
{
    return system(cmd);
}

/* Check if a file exists */
static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

/* Check if running as root */
static int is_root(void)
{
    return geteuid() == 0;
}

/* Main entry point */
int main(int argc, char *argv[])
{
    char hostname[MAX_HOSTNAME + 1];
    char username[MAX_USERNAME + 1];
    char password[MAX_INPUT];
    char password_confirm[MAX_INPUT];
    char timezone[MAX_INPUT];
    char input[MAX_INPUT];
    
    const int total_steps = 4;
    
    print_banner();
    
    /* Check for root privileges */
    if (!is_root()) {
        fprintf(stderr, COLOR_YELLOW "Warning: Running without root privileges.\n");
        fprintf(stderr, "Some operations may fail. Run with sudo for full functionality.\n" COLOR_RESET);
        printf("\nContinue anyway? [y/N]: ");
        
        if (fgets(input, sizeof(input), stdin) == NULL || 
            (input[0] != 'y' && input[0] != 'Y')) {
            printf("Exiting.\n");
            return 1;
        }
    }
    
    printf("Welcome to RayuOS! Let's set up your system.\n\n");
    
    /* Step 1: Hostname */
    print_step(1, total_steps, "Set hostname");
    printf("The hostname identifies your computer on the network.\n");
    
    while (1) {
        read_line("Enter hostname [rayuos]: ", hostname, sizeof(hostname));
        
        if (strlen(hostname) == 0) {
            strcpy(hostname, "rayuos");
        }
        
        if (validate_hostname(hostname)) {
            break;
        }
        printf(COLOR_YELLOW "Invalid hostname. Use only letters, numbers, and hyphens.\n" COLOR_RESET);
    }
    
    printf("Hostname: %s\n\n", hostname);
    
    /* Step 2: Create user */
    print_step(2, total_steps, "Create user account");
    printf("Create your personal user account.\n");
    
    while (1) {
        read_line("Enter username: ", username, sizeof(username));
        
        if (validate_username(username)) {
            break;
        }
        printf(COLOR_YELLOW "Invalid username. Use lowercase letters, numbers, underscore, or hyphen.\n");
        printf("Must start with a lowercase letter.\n" COLOR_RESET);
    }
    
    while (1) {
        read_password("Enter password: ", password, sizeof(password));
        read_password("Confirm password: ", password_confirm, sizeof(password_confirm));
        
        if (strcmp(password, password_confirm) == 0) {
            if (strlen(password) >= 4) {
                break;
            }
            printf(COLOR_YELLOW "Password too short (minimum 4 characters).\n" COLOR_RESET);
        } else {
            printf(COLOR_YELLOW "Passwords do not match. Try again.\n" COLOR_RESET);
        }
    }
    
    printf("User: %s\n\n", username);
    
    /* Step 3: Timezone */
    print_step(3, total_steps, "Set timezone");
    printf("Common timezones: UTC, America/New_York, Europe/London, Asia/Tokyo\n");
    
    while (1) {
        read_line("Enter timezone [UTC]: ", timezone, sizeof(timezone));
        
        if (strlen(timezone) == 0) {
            strcpy(timezone, "UTC");
        }
        
        char tzpath[256];
        snprintf(tzpath, sizeof(tzpath), "/usr/share/zoneinfo/%s", timezone);
        
        if (file_exists(tzpath)) {
            break;
        }
        printf(COLOR_YELLOW "Invalid timezone. Check /usr/share/zoneinfo/ for valid values.\n" COLOR_RESET);
    }
    
    printf("Timezone: %s\n\n", timezone);
    
    /* Step 4: Apply configuration */
    print_step(4, total_steps, "Applying configuration...");
    
    printf("  Setting hostname... ");
    if (set_hostname(hostname) == 0) {
        printf(COLOR_GREEN "OK\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW "WARN\n" COLOR_RESET);
    }
    
    printf("  Creating user... ");
    if (create_user(username, password) == 0) {
        printf(COLOR_GREEN "OK\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW "WARN\n" COLOR_RESET);
    }
    
    printf("  Setting timezone... ");
    if (set_timezone(timezone) == 0) {
        printf(COLOR_GREEN "OK\n" COLOR_RESET);
    } else {
        printf(COLOR_YELLOW "WARN\n" COLOR_RESET);
    }
    
    /* Mark first boot as complete */
    FILE *f = fopen("/etc/rayuos-firstboot-done", "w");
    if (f != NULL) {
        fprintf(f, "1\n");
        fclose(f);
    }
    
    printf("\n");
    printf(COLOR_GREEN COLOR_BOLD "Setup complete!\n" COLOR_RESET);
    printf("You can now log in as '%s'.\n", username);
    printf("\nEnjoy RayuOS!\n\n");
    
    /* Clear sensitive data from memory */
    memset(password, 0, sizeof(password));
    memset(password_confirm, 0, sizeof(password_confirm));
    
    return 0;
}
