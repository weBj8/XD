#include <stdio.h>
#include <stdlib.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#define MATCH(a, b) strcmp(a, b) == 0

#define P_POWER_SAVER 0
#define P_BALANCED 1
#define P_PERFORMANCE 2

#define STR_POWER_SAVER "power-saver"
#define STR_BALANCED "balanced"
#define STR_PERFORMANCE "performance"
char* find_device(char* device){
	DIR *dir;
	int fd,rc;
	struct dirent *entry;
    struct libevdec *dev = NULL;
	char *input_dir = "/dev/input/";
	dir = opendir(input_dir);
	while((entry = readdir(dir))!=NULL){
		if (strncmp(entry->d_name,"event",5)==0){
			sprintf(device,"%s%s",input_dir,entry->d_name);
    			fd = open(device, O_RDONLY | O_NONBLOCK);
			rc = libevdev_new_from_fd(fd,&dev);
			if(rc >=0 && MATCH("Ideapad extra buttons",libevdev_get_name(dev))){
                return device;
			}
		}
	}
	closedir(dir);
    return NULL;
}

int get_state()
{
    setbuf(stdout, NULL);
    char *cmd = "powerprofilesctl get";
    char result[50] = {0};
    FILE *fp = NULL;
    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        printf("failed to run powerprofilesctl");
    }

    fread(result, sizeof(char), sizeof(result), fp);
    fclose(fp);

    result[strlen(result) - 1] = 0;

    if (MATCH(result, STR_PERFORMANCE))
    {
        return P_PERFORMANCE;
    }
    else if (MATCH(result, STR_BALANCED))
    {
        return P_BALANCED;
    }
    else if (MATCH(result, STR_POWER_SAVER))
    {
        return P_POWER_SAVER;
    }

    return -1;
}

void set_state(int state)
{
    char cmd[50] = "powerprofilesctl set ";
    char *state_str = NULL;
    switch (state)
    {
    case P_POWER_SAVER:
        state_str = STR_POWER_SAVER;
        break;
    case P_BALANCED:
        state_str = STR_BALANCED;
        break;
    case P_PERFORMANCE:
        state_str = STR_PERFORMANCE;
        break;
    default:
        state_str = NULL;
        break;
    }

    strcat(cmd,state_str);
    system(cmd);
}

void toggle()
{
    int state = get_state() + 1;
    if (state > 2)
    {
        state = 0;
    }
    set_state(state);
}

int main()
{
    char device[50];
    find_device(device);

    struct libevdev *dev = NULL;
    int fd;
    int rc = 1;

    fd = open(device, O_RDONLY | O_NONBLOCK);
    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0)
    {
        fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
        exit(1);
    }

    do
    {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0 && MATCH(libevdev_event_code_get_name(ev.type, ev.code), "MSC_SCAN") && ev.value == 65)
        {
            toggle();
        }
    } while (rc == 1 || rc == 0 || rc == -EAGAIN);
}
