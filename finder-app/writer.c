#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
    // Replace printf with syslog

    openlog("writer app", LOG_PID | LOG_CONS, LOG_USER);
    if (argc != 3)
    {
        syslog(LOG_ERR, "use: ./writer writefile writestr");
        syslog(LOG_ERR, "writefile: the path including the filename");
        syslog(LOG_ERR, "writestr the string to be written");
        return 1;
    }
    syslog(LOG_ERR, "Opening file %s", argv[1]);
    int fd = creat(argv[1], 0644);    
    if (fd == 1)
        syslog(LOG_ERR, "Unable to open file: %s", argv[1]);
    else{
    
        syslog(LOG_DEBUG, "writing string <%s> to file <%s>", argv[2], argv[1]);
        int len = strlen(argv[2]);
        ssize_t ret;
        while (len != 0 && (ret = write (fd, argv[2], len)) != 0)
        {
            if (ret == -1) {
                if (errno == EINTR)
                    continue;
                syslog(LOG_ERR, "%s","write");
                break;
            }
            len -= ret;
            argv[2] += ret; 
        }
    }
    syslog(LOG_DEBUG, "Closing file %s", argv[1]);
    close(fd);
    closelog();
return 0; 
}
