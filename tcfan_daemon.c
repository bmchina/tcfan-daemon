#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define FAN 15
#define INIT_LEVEL 0
#define MAX_SIZE 32   
#define TEMP_FILE "/sys/class/thermal/thermal_zone0/temp"


char buffer[MAX_SIZE];

int level = INIT_LEVEL;
int gpio = FAN;
double start = 36;
int autoAdjust = 0;
int speed = 9;
char *mainProcess;
char *tempFile = {TEMP_FILE};

static double temperature = 0;

int speedArray[9]={0,1,20,40,50,60,80,99,100};

//pthread_t threadTempMonitor;
//pthread_t threadPmwGenerator;

void *tempMonitor(void *tid);
void *pwmGenerator(void *tid);
void startTempMonitor();
void startPwmGenerator();

void abnormalStop(int signo);
void show_help(char *name);
int writeLog(double a, int b, double c);
int showTime();
int parse_options(int argc, char *argv[]);


int main(int argc, char *argv[])
{
    int temp[9];
    mainProcess = argv[0];
    parse_options(argc,argv);
    if(start <= 0.00 || start > 100.00){
        fprintf(stderr, "%s: Temperature range: 1-100.\n", mainProcess);
	exit(0);
    }
    if(gpio <= 0 || gpio > 1024){
        fprintf(stderr, "%s: GPIO range: 1-1024.\n", mainProcess);
	exit(0);
    }
    if(speed <= 0 || speed > 9 ){
        fprintf(stderr, "%s: Fan speed range: 1-9.\n", mainProcess);
	exit(0);
    }
    if(argv[optind] != NULL)
        tempFile = argv[optind];

    if(level == 1){
	
        for(int i=0,j=8; i<=8,j>=0; i++,j--)
           temp[i] = speedArray[j];	
	memcpy(speedArray,temp,sizeof(speedArray));
    }
    
    
    printf("CPU_TEMP:%.2f\n", temperature);
    printf("PIN_LEVEL:%d\n", level);
    printf("GPIO_NO:%d\n", gpio);
    printf("START_TEMP:%.2f\n", start);
    printf("FAN_SPEED:%d\n", speed);
    printf("PNP_SPEED:%d\n",speedArray[speed-1]);
    printf("EXT_FILE:%s\n", argv[optind]);
    printf("TEMP_FILE:%s\n", tempFile);
    

    signal(SIGINT, abnormalStop);
    signal(SIGTERM, abnormalStop);

    startTempMonitor();
    startPwmGenerator();
    
    return 0;
}


void startTempMonitor(){
    pthread_t threadTempMonitor;
    pthread_create(&threadTempMonitor, 0, &tempMonitor, NULL);
    //pthread_join(threadTempMonitor, 0);
}
void *tempMonitor(void *tid){

    while(1){  
        int file = open(tempFile, O_RDONLY);  
        if (file < 0) {
            fprintf(stderr, "%s: failed to open temperature file.\n", mainProcess);
	    pthread_exit(0);  
	}
        if (read(file, buffer, MAX_SIZE) < 0) {  
            fprintf(stderr, "%s: failed to read temperature.\n", mainProcess);
	    pthread_exit(0);
	}
        close(file);
        temperature = atoi(buffer) / 1000.0;
	delay(1000);
    }

}


void startPwmGenerator(){
    pthread_t threadPwmGenerator;
    pthread_create(&threadPwmGenerator, 0, &pwmGenerator, NULL);
    pthread_join(threadPwmGenerator, 0);
    //pthread_cancel(threadPwmGenerator);
    startPwmGenerator();
}

void *pwmGenerator(void *tid){
    int currentSpeed;

    if (wiringPiSetup() == -1){
        printf("%s: setup GPIO error!\n", mainProcess);
        exit(0);
    }
    
    if(temperature < (double)start)
	currentSpeed = 1;
    if(temperature >= (double)start)
	currentSpeed = speed;
    
    if(autoAdjust != 0){	    
    
        if(temperature < (double)(start))
	    currentSpeed = 0;
	if(temperature >= (double)(start))
	    currentSpeed = 5;
        if(temperature >= (double)(start+5))
	    currentSpeed = 6;
        if(temperature >= (double)(start+10))
	    currentSpeed = 7;
        if(temperature >= (double)(start+15))
	    currentSpeed = 8;
        if(temperature >= (double)(start+20))
	    currentSpeed = 9;

    }

    softPwmCreate(gpio, speedArray[currentSpeed-1], 100);
    
    if(writeLog(temperature, currentSpeed, start))
        exit(-1);
 
    showTime();
    printf("Temperature: %.2f, Speed: %d, Target: %.2f.\n", temperature, currentSpeed, start);
    delay(10000);
    softPwmStop(gpio);
    digitalWrite(gpio, level);
    
}


void abnormalStop(int signo) 
{
    switch(signo){
        
	case SIGINT:
            printf("\nSTOP: Program terminated by Ctrl+C.\n");
            softPwmStop(gpio);
            digitalWrite(gpio, level);
            break;

        case SIGTERM:
            printf("\nSTOP: Program killd by system.\n");
            softPwmStop(gpio);
            digitalWrite(gpio, level);
            break;
        
	default:
            printf("\nSTOP: Program abnormally terminated.\n");
            softPwmStop(gpio);
            digitalWrite(gpio, level);
	    break;

    }
    exit(0);
}


int parse_options(int argc, char *argv[]){

    int ch;

    while ((ch = getopt(argc, argv, "hvRAg:t:s:")) != -1){

        switch (ch) {

            case 'v':
                printf("\nVersion: 0.9.9  Copyleft by BH4DKU  2022/08/08\n\n");   
		exit(0);
		break;
            
	    case 'h':
                show_help(argv[0]);
		exit(0);
		break;
            
	    case 'g':
                gpio = atoi(optarg);
		break;

            case 's':
                speed = (int)atoi(optarg);
                break;

	    case 't':
                start = atof(optarg);
                break;

	    case 'A':
                autoAdjust = 1;
                break;

	    case 'R':
                level = !(INIT_LEVEL);
                break;

	    default:
		//fprintf(stderr,"bad syntax, for help run with \"-h\".\n");
		exit(0);	
		break;
        }
    }
}


void show_help(char *name){
    fprintf(stderr,
	"\n"
	"Usage: %s [-gtsRvh] [filename]\n"
	"	-g pin		GPIO output pin number(WiringPi format ONLY)\n"
	"	-t numberic	Fan start temperature(1.00-99.99)\n"
	"	-s numberic	Fan speed(1-9, 9 means full speed)\n"
	"	-A 		Auto adjust fan speed('-s' ignored)\n"
	"	-R 		Reverse level voltage(to control PNP MOSFET)\n"
	"	-v		Show version informations\n"
	"	-h		Show this help\n"
	"\n"
	"	filename	Linux CPU temperature file\n"
	"			default:/sys/class/thermal/thermal_zone0/temp)\n"
	"\n"
	"Note:\n"
	"	This program work under default settings as below:\n"
	"	 ONLY tested on NANOPI Duo2(without PWM output pin)\n"
	"	 level -- default for high level to control PNP\n"
	"	 temperature -- please specify celsius degree\n"
	"\n"
	"Examples:\n"
	"	Get help information:\n"
	"	    %s -h\n"
	"\n"
	"	Show version infomation:\n"
	"	    %s -v\n"
	"\n"
	"	Start TCFAN daemon:\n"
	"	    %s -R -s 45 -g 15 /sys/class/thermal/thermal_zone0/temp\n"
	"\n"
        "Version: 0.9.9  Copyleft by BH4DKU  2022/08/08\n" 
	"\n",
		name,
		name,
		name,
		name
    );
}

int writeLog(double a, int b, double c){

    FILE * log;
    char*wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    time_t t;
    struct tm *p;
    t = time(NULL);
    //1970.1.1
    //p = gmtime(&t);
    p = localtime(&t);
    log = fopen("/var/log/pi-star/tcfan.log", "a");
    fprintf(log, "%d.%02d.%02d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
    fprintf(log, " %s ", wday[p->tm_wday]);
    fprintf(log, "%02d:%02d:%02d --> ", p->tm_hour,p->tm_min, p->tm_sec);
    fprintf(log, "Temperature: %.2f, Speed: %d, Target: %.2f.\n", a, b, c);
    fclose(log);
    return(0);
}
int showTime(){

    char*wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    time_t t;
    struct tm *p;
    t = time(NULL);
    //p = gmtime(&t);
    p = localtime(&t);
    printf("%d.%02d.%02d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
    printf(" %s ", wday[p->tm_wday]);
    printf("%02d:%02d:%02d --> ", p->tm_hour,p->tm_min, p->tm_sec);
    return(0);
}
