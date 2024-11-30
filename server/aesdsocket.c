#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT "9000"
#define BACKLOG 50
#define BUFFER_SIZE 1024

int log_file_fd = -1;
const char *log_file_path = "/tmp/server.log";

// Signal handler for graceful shutdown
static void handle_signal(int signal) {
    if (log_file_fd >= 0)
        close(log_file_fd);
    unlink(log_file_path);
    syslog(LOG_DEBUG, "Signal caught, exiting");
    closelog();
    _exit(0);
}

int main(int argc, char **argv) {
    openlog(NULL, 0, LOG_USER);

    // Setup signal handling for SIGINT and SIGTERM
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;

    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Failed to set up signal handlers");
        exit(1);
    }

    // Configure address for the server
    struct addrinfo hints, *addr_list, *addr_ptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, PORT, &hints, &addr_list) != 0) {
        syslog(LOG_ERR, "Failed to get address info");
        exit(1);
    }

    // Attempt to bind to an available address
    int server_fd, optval = 1;
    for (addr_ptr = addr_list; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next) {
        server_fd = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);
        if (server_fd == -1)
            continue;

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            syslog(LOG_ERR, "Failed to set socket options");
            exit(1);
        }

        if (bind(server_fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen) == 0)
            break;

        close(server_fd);
    }

    if (addr_ptr == NULL) {
        syslog(LOG_ERR, "Failed to bind socket");
        freeaddrinfo(addr_list);
        exit(1);
    }

    freeaddrinfo(addr_list);

    // Start listening for incoming connections
    if (listen(server_fd, BACKLOG) == -1) {
        syslog(LOG_ERR, "Failed to listen on socket");
        exit(1);
    }

    // Daemonize the process if "-d" flag is provided
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        pid_t pid = fork();
        if (pid == -1) {
            syslog(LOG_ERR, "Fork failed");
            exit(1);
        }
        if (pid != 0)
            exit(0);

        if (setsid() == -1) {
            syslog(LOG_ERR, "Failed to create new session");
            exit(1);
        }

        pid = fork();
        if (pid == -1) {
            syslog(LOG_ERR, "Second fork failed");
            exit(1);
        }
        if (pid != 0)
            exit(0);

        umask(0);
        chdir("/");
        for (int fd = 0; fd <= STDERR_FILENO; fd++)
            close(fd);

        int dev_null_fd = open("/dev/null", O_RDWR);
        dup2(dev_null_fd, STDIN_FILENO);
        dup2(dev_null_fd, STDOUT_FILENO);
        dup2(dev_null_fd, STDERR_FILENO);

        syslog(LOG_DEBUG, "Server daemonized");
    }

    // Main server loop
    struct sockaddr_storage client_addr;
    char client_host[NI_MAXHOST], client_service[NI_MAXSERV];
    char recv_buffer[BUFFER_SIZE], file_buffer[BUFFER_SIZE];
    int recv_len = 0;

    while (1) {
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1) {
            syslog(LOG_WARNING, "Failed to accept connection");
            continue;
        }

        if (getnameinfo((struct sockaddr *)&client_addr, addr_len, client_host, NI_MAXHOST, client_service, NI_MAXSERV, 0) == 0) {
            syslog(LOG_INFO, "Connection accepted from (%s, %s)", client_host, client_service);
        } else {
            syslog(LOG_INFO, "Connection accepted from unknown client");
        }

        // Process client data
        while (1) {
            if (log_file_fd < 0) {
                log_file_fd = open(log_file_path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                if (log_file_fd == -1) {
                    syslog(LOG_ERR, "Failed to open log file");
                    exit(1);
                }
            }

            int bytes_read = read(client_fd, recv_buffer + recv_len, BUFFER_SIZE - recv_len);
            if (bytes_read <= 0)
                break;

            char *newline_ptr = memchr(recv_buffer, '\n', recv_len + bytes_read);
            if (!newline_ptr) {
                write(log_file_fd, recv_buffer, recv_len + bytes_read);
                recv_len = 0;
            } else {
                write(log_file_fd, recv_buffer, newline_ptr - recv_buffer + 1);
                close(log_file_fd);
                recv_len = (recv_buffer + recv_len + bytes_read) - (newline_ptr + 1);
                if (recv_len > 0)
                    memmove(recv_buffer, newline_ptr + 1, recv_len);

                log_file_fd = open(log_file_path, O_RDONLY);
                if (log_file_fd == -1) {
                    syslog(LOG_ERR, "Failed to reopen log file for reading");
                    exit(1);
                }

                while ((bytes_read = read(log_file_fd, file_buffer, BUFFER_SIZE)) > 0) {
                    write(client_fd, file_buffer, bytes_read);
                }
                close(log_file_fd);
                log_file_fd = -1;
            }
        }

        close(client_fd);
        close(log_file_fd);
        log_file_fd = -1;
        syslog(LOG_INFO, "Connection closed");
    }

    closelog();
    return 0;
}
