#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

static long get_nanos(void) {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int main()
{
	int fd;
	char zero = '0';
	char one = '1';
	char value[1];
	int try = 0, i = 0, flag_repeat = 0, flag_correct = 0;
	int count_f = 0;
	struct timespec ts;
	long nanos;
	long start;
	long last;
	long pwm_off_time, pwm_on_time;
	float hertz = 0.00000000000000, freq = 0;
	char* values[] = {"D1", "D9", "DA", "DB", "DC", "DD", "DE", "DF", "E0", "E1"};
	char* opts[] = {"-a", "-h", "-l"};
	const char* prescale = "./set_prescalar.sh ";
	const char* list_all = "ls ";
	char command[250];

	if((fd = open("/sys/class/gpio/gpio461/value",O_RDONLY)) < 0) {
		perror("open");
		exit(errno);
	}
	
	start = get_nanos();
	//last = start;
	printf("Start time: %lu \n", start);
	while(1) {
		read(fd,&value,1);
		if(value[0] == '1') {
			if(count_f == 0) {
				nanos = get_nanos();
				pwm_off_time = nanos - last;
				//printf("T-off = %lu \n", pwm_off_time);
				last = nanos;
			}
			count_f = 1;
		}
		else if(value[0] == '0') {
			if(count_f == 1) {
				nanos = get_nanos();
				pwm_on_time = nanos - last;
				//printf("T-on = %lu \n", pwm_on_time);
				hertz = 1 / (float)(pwm_off_time + pwm_on_time);
				freq = (hertz * pow(10,9));
				printf("Freq = %lf Hz\n", freq);
				if(freq > 30.00) {
					printf("Greater than 30 \n");
						
					sprintf(command, "%s", prescale);
					strcat(command, values[i]);

					system(command);
					printf("\n");
					i++;
					flag_repeat = 0;
					flag_correct = 0;
				}
				else if (freq < 29.5 && flag_repeat > 2) {
					printf("Lesser than 29.5 \n");
					//i = 0;						
					sprintf(command, "%s", prescale);
					strcat(command, values[i]);

					system(command);
					printf("\n");
					flag_repeat++;

				}
				else {
					printf("Correct value = %s ", values[i]);
					flag_repeat++; flag_correct++;
					if (flag_correct > 10) {
						//i++;
						//flag_repeat = 0;
						goto exit;
					}
				}
				//	goto exit;
				last = nanos;
			}
			count_f = 0;
		}
		
		//flag_repeat++;
		lseek(fd,-1,SEEK_CUR);
		if(i > 10)
			i = 10;
	}
exit:
	close(fd);
	return 0;
}
