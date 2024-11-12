#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <wiringPi.h>

#define PINSS	17

#define FFMPEG_BIN  "/usr/bin/ffmpeg"
#define V4L_DEV     "/dev/video0"
#define ALSA_CARD   "hw:CARD=DVC100,DEV=0"

int cid;

void
tape_start() {
    printf("TAPE START\n");

    cid = fork();
    if (cid < 0) {
        fprintf(stderr, "fork failed\n");
    } else if (cid == 0) {
        /* child */
        char output[1024];
        getcwd(output, 1024);
        strcat(output, "/Videos/"); /* i dont fucking care */
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char fname[256];
        snprintf(fname, 256, "%d-%02d-%02d_%02d:%02d:%02d.mkv",
            tm.tm_year + 1990, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_mon, tm.tm_sec);
        strcat(output, fname);
        /* ffmpeg -f alsa -ac 2 -ar 48000 -thread_queue_size 1024 -i $alsa_card -f v4l2 -i $v4l_dev -c:a pcm_s24le -c:v libx264 -b:v 2M -preset fast $output_dir/$fname */
        char *args[] = { FFMPEG_BIN, "-f", "alsa", "-ac", "2", "-ar", "48000",
            "-thread_queue_size", "1024", "-i", ALSA_CARD,
            "-f", "v4l2", V4L_DEV, "-c:a", "pcm_s24le", "-c:v", "libx264"
            "-b:v", "2M", "-preset", "fast", output, NULL };

        if (execve(FFMPEG_BIN, args, NULL) < 0) {
            fprintf(stderr, "execve failed: %s\n", strerror(errno));
            exit(1);
        }
    } else {
        /* parent */
    }
}

void
tape_end() {
    printf("TAPE STOP\n");
    if (kill(-cid, SIGKILL) < 0) {
        fprintf(stderr, "kill error: %s\n", strerror(errno));
    }
}

int
main(int argc, char **argv) {
    wiringPiSetupGpio();
    pinMode(PINSS, INPUT);
    pullUpDnControl(PINSS, PUD_DOWN);

    // poll
    int sss_, sss = 0;
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

