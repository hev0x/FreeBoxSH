#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include <dirent.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"


static const char *TAG = "SPIFFS";
static const char *SECRET = "root";

bool* auth_flag = false;

#define LINE_MAX	100
char actual_path[256];


void mount_spiffs(){

    // SPIFFS init
    printf("\n[+] SPI Flash File System initiation: \n");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    esp_vfs_spiffs_register(&conf);
    size_t total = 0, used = 0;
    esp_spiffs_info(conf.partition_label, &total, &used);
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	printf("\nWelcome to FreeBox Console v0.1 \n");    

	strcpy(actual_path, "/spiffs");

/*  HOLD 
    // Unmount partition and disable SPIFFS
    printf("\nSPIFFS system unmount:\n");
    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "SPIFFS unmounted");

*/
}


void menu_help()
{
    printf("\r\n\r\nhelp: \n"
           "   Lists all the registered commands\n"
        "\r\nls                                List files in specified directory\n"
        "cd                                Move to specified directory\n"
        "cat <file>                        Get contents of specified file\n"
        "touch <file> -w <filecontent>     Create a file and write content to it\n"
        "delete <file>                     Delete the specified file\n"
        "hardware                          Get information hardware\n"
        "memory                            Get current memory information\n"
        "restart                           Restart CPU\n"
        "password                          Enter password for authentication\n"
        "netinfo (HOLD)                                                       \n"
        "ps (HOLD)\n"
        "read <GPIO> (HOLD)\n"
        "write <GPIO> (HOLD)\n" 
        "set <GPIO> (HOLD)");
    printf("\r\n");

}

void memoryView(){

	int heapMemory = (xPortGetFreeHeapSize())/1024;
	int freeBlock = (heap_caps_get_largest_free_block(MALLOC_CAP_8BIT))/1024;
	int DRamMemory = (heap_caps_get_free_size(MALLOC_CAP_8BIT))/1024;
	int IRamMemory = (heap_caps_get_free_size(MALLOC_CAP_32BIT) - heap_caps_get_free_size(MALLOC_CAP_8BIT))/1024;

    printf("\r\n\r\n-> Getting ESP32 memory blocks info... \n\n ");
    printf("[+] Heap: %d kB\n ", heapMemory);
    printf("[+] DRam: %d kB\n ", DRamMemory);
    printf("[+] IRam: %d kB\n ", IRamMemory);
    printf("[+] Maximum Free Block %d kB\n ", freeBlock);
    return;

}

/**************************************************************************************************************/
void hardware_info()
{
    printf("\r\n\r\n-> Getting hardware information...\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
	printf("\r\n");
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }
    return;
}

void ls_cmd(char* path) {

	printf("\r\n\r\n-> Listing files in directory %s\r\n", path);
	
	DIR *dir;
	dir = opendir(path);
    if (!dir) {
        printf("-> Error opening directory\r\n");
        return;
    }
	
	// list the files and directories
	struct dirent *direntry;
	while ((direntry = readdir(dir)) != NULL) {
		
		if(strcmp(direntry->d_name, "/spiffs") == 0) continue;
		
		if(direntry->d_type == DT_DIR) printf("DIR\t");
		else if(direntry->d_type == DT_REG) printf("FILE\t");
		else printf("???\t");
		printf("%s\r\n", direntry->d_name);
	}
	
	// close the folder
	closedir(dir);
}

void cat_cmd(char* filename) {

    printf("\r\n");

	// open the specified file
	char file_path[300];
	strcpy(file_path, actual_path);
	strcat(file_path, "/");
	strcat(file_path, filename);

	FILE *file;
	file = fopen(file_path, "r");
    if (!file) {
        printf("\r\n\r\n-> Error opening file %s\r\n", file_path);
        return;
    }
    printf("\r\n\r\n-> Content of the file %s\r\n", filename);
	// get the file content
	int filechar;
	while((filechar = fgetc(file)) != EOF)
		putchar(filechar);
	
	// close the folder
	fclose(file);
}

void touch_cmd(char* touch, char* content_file) {

    printf("\r\n\n-> Write and create new file:\n");
    // Create a file.
    ESP_LOGI(TAG, "Opening file");

	char file_path[300];
	strcpy(file_path, actual_path);
	strcat(file_path, "/");
	strcat(file_path, touch);

	// write the specified file
	FILE *file;
	file = fopen(file_path, "w");

    fprintf(file, "%s\n",content_file);
    fclose(file);
}

void delete_cmd(char* cmddel) {

    printf("\r\n\n-> Removing selected file:\n");
    // Create a file.
    ESP_LOGI(TAG, "Deleting file");

	char file_path[300];
	strcpy(file_path, actual_path);
	strcat(file_path, "/");
	strcat(file_path, cmddel);
    remove(file_path);
}
/**************************************************************************************************************/

void cd_cmd(char* path) {

	printf("\r\n-> Moving to directory %s \r\n", path);
	
	char previous_path[256];
	strcpy(previous_path, actual_path);
	
	if(strcmp(path, "..") == 0) {
		char* pos = strrchr(actual_path, '/');
		if(pos != actual_path) pos[0] = '\0';
	}

	else if(path[0] == '/') {
	
		strcpy(actual_path, "/spiffs");
		if(strlen(path) > 1) strcat(actual_path, path);
	}
	
	else {
		strcat(actual_path, "/");
		strcat(actual_path, path);
	}
	
	DIR *dir;
	dir = opendir(actual_path);
    
	if (!dir) {
        printf("-> Directory does not exists\r\n");
        strcpy(actual_path, previous_path);
		return;
    }
	
	closedir(dir);
}



/**************************************************************************************************************/
void parse_cmd(char* command) {
	// split the command
	char* buffer;
	buffer = strtok(command, " ");
	
	if(!buffer) {
		printf("\r\n-> No command provided!\r\n");
		return;
	}
	
	// LS command, list the content of the actual directory
	if(strcmp(buffer, "ls") == 0) {

        if (auth_flag) {
            ls_cmd(actual_path);

            return;

        } else{
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");  
			return;          
        }
	}
	
	// CAT command, display the content of a file
	else if(strcmp(buffer, "cat") == 0) {
	
		char* filename = strtok(NULL, " ");

        if (auth_flag == false) {
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");      
            return;
        } 

		if(!filename) {
			printf("\r\n-> No file specified!\r\n");
			return;
		}
		cat_cmd(filename);
	}
	
	// CD command, move to the specified directory
	else if(strcmp(buffer, "cd") == 0) {
	
		char* path = strtok(NULL, " ");

        if (auth_flag == false) {
            printf("\r\n\r\n> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");    
            return;
        } 

		if(!path) {
			printf("\r\n\r\n-> No directory specified!\r\n");
			return;
		}
		cd_cmd(path);
	}
	
    // TOUCH command
    else if(strcmp(buffer,"touch") == 0){
        
        char* touch = strtok(NULL, " ");
		char* content_file = strtok(NULL, "-w ");

        if (auth_flag == false){
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");  
            return;
        } 

        if(!touch) {
            printf("\r\n\r\n-> Failed to create file\r\n");
            return;
        }
        else if(!content_file){
            printf("\r\n\r\n-> Failed to create file without content\r\n");
            return;
        }
        
		printf("\r\n\r\n%s\r\n",content_file);
        touch_cmd(touch, content_file);
    }

    // delete command
    else if(strcmp(buffer,"delete") == 0){
        
        char* cmddel = strtok(NULL, " ");

        if (auth_flag == false) {
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");    
            return;
        } 

        if(!cmddel) {
            printf("\r\n\r\n-> Failed to remove file\r\n");
            return;
        }
        delete_cmd(cmddel);
    }

    // hardware command
    else if(strcmp(buffer,"hardware") == 0){

        if (auth_flag == false) {
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");    
            return;
        } 
        hardware_info();
        return;
    }

    // help command
    else if(strcmp(buffer,"help") == 0){
        menu_help();
        return;
    }

    // ps command
    else if(strcmp(buffer,"ps") == 0){

        if (auth_flag == false) {
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");  
            return;
        }
        return;
    }

	// memory command
    else if(strcmp(buffer,"memory") == 0){

        if (auth_flag == false) {

            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");   
            return;
        } 
        memoryView();
        return;
    }

	// restart command
    else if(strcmp(buffer,"restart") == 0){

        if (auth_flag == false) {
            printf("\r\n\r\n-> Error: Not authenticated\n"
			       " Authenticate to a valid user using the 'password' command, for example:\n"
			       "Example: root@freertos(/spiffs)# password {YOUR PASSWORD HERE}\r\n");   
            return;
        } 

        for (int i = 5; i >= 0; i--) {   
        printf("\r\n\r\nRestarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
        printf("\nRestarting now.\n");
        fflush(stdout);
        esp_restart();
}

    // password command
    else if(strcmp(buffer,"password") == 0){
        
        char* passwd = strtok(NULL, " ");
        if(!passwd) {

            printf("\r\n\r\nFailed to set password user\r\n");
            return;
        }

        if(strcmp(passwd, SECRET) == 0) {

            auth_flag = true;

            if(auth_flag){
	            printf("\r\n\r\n-> Authentication success\n");               
                return;
            }

	    }else {
            auth_flag = false;
            printf("\r\n\r\nIncorrect password \r\n");
            return;
        }
	
    }

	else printf("\r\n\r\nUnknown command!\r\n");
}

/**************************************************************************************************************/
void print_prompt() {
	
	printf("\r\nroot@freertos(");
	printf(actual_path);
	printf(")# ");
	fflush(stdout);
}
/**************************************************************************************************************/

// shell task function
void shell_task(void *pvParameter) {

	// 	
	char line_ctrl[LINE_MAX];
	int line_pos = 0;

	print_prompt();

	// 
	while(1) {
	
		int c = getchar();
	
		//
		if(c < 0) {
			vTaskDelay(10 / portTICK_PERIOD_MS);
			continue;
		}
		if(c == '\r') continue;
		if(c == '\n') {
		
			line_ctrl[line_pos] = '\0';
			parse_cmd(line_ctrl);
			
			line_pos = 0;
			print_prompt();
		}
		else {
			putchar(c);
			line_ctrl[line_pos] = c;
			line_pos++;
			
			if(line_pos == LINE_MAX) {
				
				printf("\nCommand buffer full!\n");
				
				line_pos = 0;
				print_prompt();
			}
		}
	}	
}
