#include "osi_api.h"
#include "osi_log.h"
#include "app_image.h"
#include "fibo_opencpu.h"

#define MAX_FILES  3  // Number of MP3 files in the playlist
static const char *mp3_files[MAX_FILES] = {"/FFS/receivedrupees.mp3", "/FFS/nineteenrss.mp3", "/FFS/hundredrss.mp3"};
static int current_file_index = 0;  // Index of the current file being played

static uint8_t *Testbuff = NULL;

void play_next_file(void); // Forward declaration

// Callback function called after playback of a file ends
void Callplayend(void)
{
    OSI_LOGI(0, "Playback finished for file %d: %s", current_file_index, mp3_files[current_file_index]);

    // Free memory allocated for the current file
    if (Testbuff != NULL)
    {
        fibo_free(Testbuff);
        Testbuff = NULL;
    }

    // Move to the next file in the playlist
    current_file_index++;
    if (current_file_index >= MAX_FILES)
    {
        current_file_index = 0;  // Wrap back to the first file
    }

    play_next_file();  // Start playing the next file
}

// Function to play the current MP3 file
void play_next_file(void)
{
    const char *file_name = mp3_files[current_file_index];

    if (1 == fibo_file_exist(file_name))  // Check if the file exists
    {
        OSI_LOGI(0, "File exists: %s", file_name);

        // Get file size and allocate memory
        int32_t file_len = fibo_file_get_size(file_name);
        INT32 fd = fibo_file_open(file_name, O_RDONLY);
        Testbuff = (uint8_t *)fibo_malloc(file_len);
        memset(Testbuff, 0, file_len);

        // Read file data into memory and close the file
        fibo_file_read(fd, Testbuff, file_len);
        fibo_file_close(fd);

        // Start audio playback and provide callback for end of playback
        fibo_audio_mem_start(AUSTREAM_FORMAT_MP3, Testbuff, file_len, Callplayend);
    }
    else
    {
        OSI_LOGI(0, "File does not exist: %s", file_name);

        // If file doesn't exist, skip to the next file
        current_file_index++;
        if (current_file_index >= MAX_FILES)
        {
            current_file_index = 0;  // Wrap back to the first file
        }

        play_next_file();  // Attempt to play the next file
    }
}

// Main demo function to start playback
static void hello_world_demo()
{
    OSI_LOGI(0, "Application thread started.");
    set_app_ver(app_ver);

    current_file_index = 0;  // Start with the first file in the playlist
    play_next_file();

    fibo_thread_delete();  // Delete the thread after playback starts
}

// Entry point for the application
void *appimg_enter(void *param)
{
    OSI_LOGI(0, "App image enter.");
    fibo_thread_create(hello_world_demo, "mythread", 10 * 1024, NULL, OSI_PRIORITY_NORMAL);
    return NULL;
}

// Exit point for the application
void appimg_exit(void)
{
    OSI_LOGI(0, "Application image exit.");
}

