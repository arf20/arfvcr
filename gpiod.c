#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

#include <wiringPi.h>

#define PINSS       17
#define PINRECLED   27

#define FFMPEG_BIN  "/usr/bin/ffmpeg"
#define V4L_DEV     "/dev/video0"
#define ALSA_CARD   "hw:CARD=DVC100,DEV=0"

int cpid = 0;

void *
monitor() {
    char buf[512], symlnkbuf[512];
    DIR *pdir = NULL;
    struct dirent *de = NULL;

    while (1) {
        digitalWrite(PINRECLED, LOW);
        snprintf(buf, 512, "/proc/%d/fd/", cpid);
        pdir = opendir(buf);
        if (!pdir) continue;
        while ((de = readdir(pdir)) != NULL) {
            if (de->d_type == DT_LNK) {
                snprintf(buf, 512, "/proc/%d/fd/%s", cpid, de->d_name);
                readlink(buf, symlnkbuf, 512);
                if (strncmp(symlnkbuf, "/dev/video0", 11) == 0)
                    digitalWrite(PINRECLED, HIGH);
            }
        }
        closedir(pdir);
        usleep(100000);
    }
}

void
tape_start() {
    cpid = fork();
    if (cpid < 0) {
        fprintf(stderr, "fork failed\n");
    } else if (cpid == 0) {
        /* child */
        char output[1024];
        getcwd(output, 1024);
        strcat(output, "/Videos/"); /* i dont fucking care */
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char fname[256];
        snprintf(fname, 256, "%d-%02d-%02d_%02d-%02d-%02d.mkv",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_mon, tm.tm_sec);
        strcat(output, fname);
        /* ffmpeg -f alsa -ac 2 -ar 48000 -thread_queue_size 1024 -i $alsa_card -f v4l2 -i $v4l_dev -c:a pcm_s24le -c:v libx264 -b:v 5M -pix_fmt yuv420p -profile:v baseline -preset ultrafast $output_dir/$fname */
        char *args[] = { FFMPEG_BIN, "-f", "alsa", "-ac", "2", "-ar", "48000",
            "-thread_queue_size", "1024", "-i", ALSA_CARD,
            "-f", "v4l2", "-i", V4L_DEV, "-c:a", "pcm_s24le", "-c:v", "libx264",
            "-b:v", "5M", "-pix_fmt", "yuv420p", "-profile:v", "baseline", "-preset", "ultrafast", output, NULL };

        /*for (int i = 0; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");*/

        /* redirect stdout & stderr */
        int ffmpeglogfd = open("ffmpeg.log",O_WRONLY | O_CREAT | O_APPEND, 0600);
        dup2(ffmpeglogfd, STDOUT_FILENO);
        dup2(ffmpeglogfd, STDERR_FILENO);
        close(ffmpeglogfd);

        if (execve(FFMPEG_BIN, args, NULL) < 0) {
            fprintf(stderr, "execve failed: %s\n", strerror(errno));
            exit(1);
        }
    } else {
        /* parent */
    	printf("TAPE START\n");
    }
}

void
tape_end() {
    printf("TAPE STOP\n");
    if (kill(cpid, SIGINT) < 0) {
        fprintf(stderr, "kill error: %s\n", strerror(errno));
    }
}

int
main(int argc, char **argv) {
    /* pin setup */
    wiringPiSetupGpio();
    pinMode(PINSS, INPUT);
    pinMode(PINRECLED, OUTPUT);
    pullUpDnControl(PINSS, PUD_DOWN);

    pthread_t tid;
    if (pthread_create(&tid, NULL, &monitor, NULL) != 0)
        fprintf(stderr, "pthread_create failed\n");
    else
        pthread_detach(tid);

    /* poll */
    int sss_ = 0, sss = 0;
    while (1) {
        sss = digitalRead(PINSS);
        if (sss_ == 0 && sss == 1) {
            tape_start();
        } else if (sss_ == 1 && sss == 0) {
            tape_end();
        }

        sss_ = sss;
        usleep(100000);
    }
}

