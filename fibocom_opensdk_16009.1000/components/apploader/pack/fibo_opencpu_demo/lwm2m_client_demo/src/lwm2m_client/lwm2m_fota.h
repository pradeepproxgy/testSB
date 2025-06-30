#ifndef __LWM2M_FOTA__
#define __LWM2M_FOTA__

#include "fibo_opencpu.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "liblwm2m.h"
#include "app_lwm2m.h"

// Define constants for FOTA (Firmware Over The Air) update results.
#define LWM2M_FOTA_UPDATE_FAILED 2
#define LWM2M_FOTA_UPDATE_SUCCESS 1
#define LWM2M_FOTA_SUCCESS 0
#define LWM2M_FOTA_FAILED -1

// Callback function type for receiving notifications about FOTA status.
typedef int (*recv_notify_ack)(void* thi, int status);

// Enum for FOTA state, representing different stages of the FOTA process.
typedef enum {
    LWM2M_FOTA_IDLE = 0,                 // Idle state, no operation in progress.
    LWM2M_FOTA_DOWNLOADING,              // Downloading firmware package.
    LWM2M_FOTA_DOWNLOADED,               // Firmware package downloaded successfully.
    LWM2M_FOTA_UPDATING                  // Updating firmware.
} lwm2m_fota_state_e;

// Enum for FOTA update results, detailing possible outcomes of an update attempt.
typedef enum update_result {
    UPDATE_RESULT_INIT,                  // Initialization state.
    UPDATE_RESULT_SUCC,                  // Update succeeded.
    UPDATE_RESULT_NOT_ENOUGH_STORAGE,    // Insufficient storage for the new firmware.
    UPDATE_RESULT_OUT_OF_MEM,            // Out of memory during download.
    UPDATE_RESULT_LOST_CONNECT,          // Connection lost during download.
    UPDATE_RESULT_CRC_FAIL,              // CRC check failure of the downloaded package.
    UPDATE_RESULT_UNSUPPORTED_TYPE,      // Unsupported package type.
    UPDATE_RESULT_INVALID_URI,           // Invalid URI for the firmware package.
    UPDATE_RESULT_UPDATE_FAIL            // Update failed.
} update_result;

// Structure for managing FOTA operations.
typedef struct lwm2m_fota_manager_tag_s {
    uint8_t id;                          // Identifier for the FOTA manager.
    lwm2m_fota_state_e state;            // Current state of FOTA.
    int update_result;                   // Result of the last update attempt.
    lwm2m_context_t* lwm2m_context;      // Context of the LwM2M client.
    int rpt_state;                       // Reporting state.
    char* ota_uri;                       // URI for the OTA (Over The Air) update.
    UINT32 notify_ack_sem;               // Semaphore for notification acknowledgments.
    int (*download_start_callback)(void* thi, uint8_t* url, int len); // Callback when download starts.
    void (*download_event_callback)(int evid, int reason, void* arg); // Callback for download events.
} lwm2m_fota_manager;

/**
 * Destroys the FOTA manager instance, freeing all associated resources.
 * This should be called when the FOTA functionality is no longer needed.
 * 
 * @param None
 */
void lwm2m_fota_destroy(void);

/**
 * Initiates the download of a firmware update from a specified URI.
 * This function starts the download process and updates the FOTA state accordingly.
 * 
 * @param thi Pointer to the FOTA manager instance.
 * @param uri URI of the firmware to be downloaded.
 * @param len Length of the URI string.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int lwm2m_fota_start_download(lwm2m_fota_manager* thi, const char* uri, uint32_t len);

/**
 * Retrieves the current state of the FOTA process.
 * This can be used to check if a download is in progress, completed, or if an update is pending execution.
 * 
 * @param thi Pointer to the FOTA manager instance.
 * @return Returns the current FOTA state.
 */
lwm2m_fota_state_e lwm2m_fota_get_state(const lwm2m_fota_manager* thi);

/**
 * Executes the firmware update if a firmware file has been successfully downloaded.
 * This function applies the downloaded firmware to the device.
 * 
 * @param thi Pointer to the FOTA manager instance.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int lwm2m_fota_execute_update(lwm2m_fota_manager* thi);

/**
 * Retrieves the result of the last firmware update execution.
 * This can be used to determine if the update was successful or if it failed.
 * 
 * @param thi Pointer to the FOTA manager instance.
 * @return Returns the result of the update execution.
 */
int lwm2m_fota_get_update_result(const lwm2m_fota_manager* thi);

/**
 * Sets the result of the firmware update execution.
 * This is typically used internally to record the outcome of an update attempt.
 * 
 * @param thi Pointer to the FOTA manager instance.
 * @param result The result of the update execution.
 * @return None
 */
void lwm2m_fota_set_update_result(lwm2m_fota_manager* thi, int result);

/**
 * Retrieves the FOTA manager instance associated with a given LwM2M context.
 * This is used to access FOTA functionalities within the context of an LwM2M session.
 * 
 * @param contextP Pointer to the LwM2M context.
 * @return Returns a pointer to the FOTA manager instance, or NULL if not available.
 */
lwm2m_fota_manager* lwm2m_get_fota_manager(lwm2m_context_t * contextP);

/**
 * Checks the state of the firmware update process.
 * This function is used to determine if an update is in progress, completed, or pending.
 * 
 * @param None
 * @return Returns the current firmware update state.
 */
int lwm2m_firmware_update_state(void);

#endif