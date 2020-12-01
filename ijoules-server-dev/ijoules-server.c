// Server side implementation of UDP client-server model
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <malloc/_malloc.h>
#include <signal.h>
#include "/Library/Frameworks/IntelPowerGadget.framework/Headers/PowerGadgetLib.h"

#define PORT 3310
#define MAXLINE 1024

// bool retrieve_pkgenergy();
void get_sample();
void start_sampling();
void get_availables();
void print_available();
void configure(char *list, int length);

bool read_timestamp(char *response);
bool read_pkg(char *response);
bool read_core(char *response);
bool read_dram(char *response);
bool read_plateform(char *response);
bool read_all(char *response);
void sighandler(int);

// TODO Add the multi packages plateform

//Global variables
int iPackage = 0;
bool gpu = false;
bool core = false;
bool dram = false;
bool pkg = false;
bool plateform = false;

bool active_pkg = false;
bool active_core = false;
bool active_dram = false;
bool active_gpu = false;
bool active_plateform = false;

PGSampleID *oldSample, *newSample;

void get_availables()
{
    PG_IsIAEnergyAvailable(iPackage, &core);
    PG_IsDRAMEnergyAvailable(iPackage, &dram);
    PG_IsGTAvailable(iPackage, &gpu);
    PG_IsPlatformEnergyAvailable(iPackage, &plateform);
    pkg = core || gpu;
}

// Driver code
int main()
{
    //Initialisation of Intel power gadged
    oldSample = (PGSampleID *)malloc(sizeof(PGSampleID));
    newSample = (PGSampleID *)malloc(sizeof(PGSampleID));
    int iPackage = 0;
    bool verify = false;
    signal(SIGTERM, sighandler);
    // signal(SIGTERM, SIG_IGN);
    PG_Initialize();
    start_sampling();
    int server_fd, new_socket, valread;
    int opt = 1;
    int sockfd;
    char buffer[MAXLINE];
    char response[MAXLINE] = "";
    struct sockaddr_in servaddr, cliaddr;
    uint len, n;
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // get Available packages ;s
    get_availables();

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket with the server address
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&servaddr,
                             (socklen_t *)&servaddr)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    len = sizeof(cliaddr); //len is value/resuslt
    for (;;)
    {
        memset(buffer, 0, MAXLINE);
        memset(response, 0, MAXLINE);

        // n = recvfrom(sockfd, (char *)buffer, MAXLINE,
        //              MSG_WAITALL, (struct sockaddr *)&cliaddr,
        //              &len);

        n = read(new_socket, buffer, 1024);
        buffer[n] = '\0';

        if (strcmp(buffer, "start") == 0)
        {
            start_sampling();
            // read_timestamp(response);
            sprintf(response, "recording started");
            send(new_socket, response, strlen(response), 0);
            continue;
        }

        if (strcmp(buffer, "exit") == 0)
        {
            free(oldSample);
            free(newSample);
            PG_Shutdown();
            close(server_fd);
            exit(0);
        }

        if (strcmp(buffer, "list_available") == 0)
        {
            print_available(response);
            // sendto(sockfd, (const char *)response, strlen(response), MSG_SEND, (const struct sockaddr *)&cliaddr,
            //        len);
            send(new_socket, response, strlen(response), 0);
            continue;
        }

        // TODO add the configuration request
        get_sample();

        if (strcmp(buffer, "timestamp") == 0)
        {
            read_timestamp(response);
        }
        if (strcmp(buffer, "pkg") == 0)
        {
            verify = read_pkg(response);
            if (!verify)
            {
                sprintf(response, "error -2");
            }
        }
        if (strcmp(buffer, "core") == 0)
        {
            verify = read_core(response);
            if (!verify)
            {
                sprintf(response, "error -2");
            }
        }
        if (strcmp(buffer, "dram") == 0)
        {
            verify = read_dram(response);
            if (!verify)
            {
                sprintf(response, "error -2");
            }
        }
        if (strcmp(buffer, "platform") == 0)
        {
            verify = read_plateform(response);
            if (!verify)
            {
                sprintf(response, "error -2");
            }
        }
        if (strcmp(buffer, "all") == 0)
        {
            verify = read_all(response);
            if (!verify)
            {
                sprintf(response, "error -2");
            }
        }

        send(new_socket, response, strlen(response), 0);
        // sendto(sockfd, (const char *)response, strlen(response), MSG_SEND, (const struct sockaddr *)&cliaddr,
        //        len);
    }

    return 0;
}

void print_available(char *response)
{
    get_availables();
    sprintf(response, "plateform:%d,pkg:%d,cores:%d,dram:%d,gpu:%d", plateform, pkg, core, dram, gpu);
}

void get_sample()
{
    // PGSampleID *temp;

    // temp = oldSample;
    // oldSample = newSample;
    // newSample = temp;
    PGSample_Release(*newSample);
    PG_ReadSample(iPackage, newSample);
}

void start_sampling()
{
    // star
    // temp = oldSample;
    // oldSample = newSample;
    PGSample_Release(*oldSample);
    PG_ReadSample(iPackage, oldSample);
}

void configure(char *list, int length)
{
    char delim[] = ":";

    char *ptr = strtok(list, delim);

    while (ptr != NULL)
    {
        if (strcmp(ptr, "plateform") == 0)
        {
            active_plateform = true;
            continue;
        }
        if (strcmp(ptr, "pkg") == 0)
        {
            active_pkg = true;
            continue;
        }
        if (strcmp(ptr, "core") == 0)
        {
            active_core = true;
            continue;
        }
        if (strcmp(ptr, "dram") == 0)
        {
            active_dram = true;
            continue;
        }
        if (strcmp(ptr, "gpu") == 0)
        {
            active_gpu = true;
            continue;
        }

        ptr = strtok(NULL, delim);
    }

    get_availables();

    active_pkg = pkg && active_pkg;
    active_core = core && active_core;
    active_dram = dram && active_dram;
    active_gpu = gpu && active_gpu;
    active_plateform = plateform && active_plateform;
}

bool read_timestamp(char *response)
{
    uint32_t seconds = 0, nanoseconds = 0;
    bool verify = PGSample_GetTime(*newSample, &seconds, &nanoseconds);
    // double timestamp = seconds + nanoseconds / 1000000000;
    sprintf(response, "timestamp_s:%u timestamp_ns:%u", seconds, nanoseconds);
    return verify;
}

bool read_pkg(char *response)
{
    double power = 0, energy = 0;
    if (!pkg)
        return true;
    bool verify = PGSample_GetPackagePower(*oldSample, *newSample, &power, &energy);
    sprintf(response, "pkg_j:%.6f ", energy);
    return verify;
}

bool read_core(char *response)
{
    double power = 0, energy = 0;
    if (!core)
        return true;
    bool verify = PGSample_GetIAPower(*oldSample, *newSample, &power, &energy);
    sprintf(response, "core_j:%.6f ", energy);
    return verify;
}

bool read_dram(char *response)
{
    double power = 0, energy = 0;
    if (!dram)
        return true;
    bool verify = PGSample_GetDRAMPower(*oldSample, *newSample, &power, &energy);
    sprintf(response, "dram_j:%.6f ", energy);
    return verify;
}

bool read_plateform(char *response)
{
    double power = 0, energy = 0;
    if (!plateform)
        return true;
    bool verify = PGSample_GetPlatformPower(*oldSample, *newSample, &power, &energy);
    sprintf(response, "platform_j:%.6f ", energy);
    return verify;
}

bool read_all(char *response)
{
    int lenght = strlen(response);
    bool verify;
    verify = read_pkg(response + lenght) && verify;
    lenght = strlen(response);
    verify = read_core(response + lenght) && verify;
    lenght = strlen(response);
    verify = read_dram(response + lenght) && verify;
    lenght = strlen(response);
    verify = read_timestamp(response + lenght);

    return verify;
}

void sighandler(int signum)
{
    if (signum == SIGTERM)
    {
        printf("closing the server ..\n");
        free(oldSample);
        free(newSample);
        PG_Shutdown();
        // close(server_fd);
        exit(0);
    }
}