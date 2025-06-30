/*
* Copyright (c) CompanyNameMagicTag
* Description: The interface and processing programs of object_firmware
*/

/*
 * This object is single instance only, and provide firmware upgrade
 * functionality. Object ID is 5.
 */

/*
 * resources:
 * 0 package                   write
 * 1 package url               write
 * 2 update                    exec
 * 3 state                     read
 * 4 update supported objects  read/write
 * 5 update result             read
 */
#include "fibo_opencpu.h"
#include "liblwm2m.h"
#include "lwm2m_fota.h"

#define CHECK_ONLY(condition, err_code) \
    do                                  \
    {                                   \
        if (condition)                  \
        {                               \
            ret = err_code;             \
            goto error;                 \
        }                               \
    } while (0);

// ---- private object "Firmware" specific defines ----
// Resource Id's:
#define RES_M_PACKAGE 0
#define RES_M_PACKAGE_URI 1
#define RES_M_UPDATE 2
#define RES_M_STATE 3
#define RES_O_UPDATE_SUPPORTED_OBJECTS 4
#define RES_M_UPDATE_RESULT 5
#define RES_O_PKG_NAME 6
#define RES_O_PKG_VERSION 7
#define RES_O_FIRMWARE_UPDATE_PROTOCOL_SUPPORT 8
#define RES_O_FIRMWARE_UPDATE_DELIVER_METHOD 9

uint8_t prv_firmware_discover(lwm2m_context_t * contextP, uint16_t instance_id, int* num_data, lwm2m_data_t** data_array, lwm2m_object_t* object)
{
    // this is a single instance object
    if (instance_id != 0) {
        return COAP_404_NOT_FOUND;
    }

    uint8_t result = COAP_205_CONTENT;
    int i;

    // is the server asking for the full object ?
    if (*num_data == 0) {
        uint16_t reslist[] = {
            RES_M_PACKAGE, RES_M_PACKAGE_URI, RES_M_UPDATE, RES_M_STATE, RES_M_UPDATE_RESULT, RES_O_PKG_NAME,
            RES_O_PKG_VERSION, RES_O_FIRMWARE_UPDATE_PROTOCOL_SUPPORT, RES_O_FIRMWARE_UPDATE_DELIVER_METHOD,
        };
        int nbres = sizeof(reslist) / sizeof(uint16_t);

        *data_array = lwm2m_data_new(nbres);
        if (*data_array == NULL)
            return COAP_500_INTERNAL_SERVER_ERROR;
        *num_data = nbres;
        for (i = 0; i < nbres; i++) {
            (*data_array)[i].id = reslist[i];
        }
    } else {
        for (i = 0; i < *num_data && result == COAP_205_CONTENT; i++) {
            switch ((*data_array)[i].id) {
                case RES_M_PACKAGE:
                case RES_M_PACKAGE_URI:
                case RES_M_UPDATE:
                case RES_M_STATE:
                case RES_M_UPDATE_RESULT:
                case RES_O_PKG_NAME:
                case RES_O_PKG_VERSION:
                case RES_O_FIRMWARE_UPDATE_PROTOCOL_SUPPORT:
                case RES_O_FIRMWARE_UPDATE_DELIVER_METHOD:
                    break;
                default:
                    result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

static uint8_t fota_uri_read(lwm2m_data_t** data_array, int item)
{
    const char* pkg_uri = NULL;
    // pkg_uri = lwm2m_fota_manager_get_pkg_uri(atiny_get_fota_manager(platform));
    if (pkg_uri == NULL) {
        pkg_uri = "";
    }
    lwm2m_data_encode_nstring(pkg_uri, strlen(pkg_uri) + 1, *data_array + item);
    return COAP_205_CONTENT;
}

static uint8_t fota_state_read(lwm2m_context_t * contextP, lwm2m_data_t* data_array)
{
    int state;
    lwm2m_fota_manager* manager = lwm2m_get_fota_manager(contextP);
    if (manager == NULL) {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    state = lwm2m_fota_get_state(manager);

    lwm2m_data_encode_int(state, data_array);
    return COAP_205_CONTENT;
}

static uint8_t fota_update_result_read(lwm2m_context_t * contextP, lwm2m_data_t* data_array)
{
    lwm2m_fota_manager* fota_manager = lwm2m_get_fota_manager(contextP);
    if (fota_manager == NULL) {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    int ret = lwm2m_firmware_update_state();
    if(ret == LWM2M_FOTA_UPDATE_SUCCESS)
    {
        lwm2m_fota_set_update_result(fota_manager, UPDATE_RESULT_SUCC);
    }
    else if(ret == LWM2M_FOTA_UPDATE_FAILED)
    {
        lwm2m_fota_set_update_result(fota_manager, UPDATE_RESULT_UPDATE_FAIL);
    }

    int update_result = lwm2m_fota_get_update_result(fota_manager);

    APP_LOG_INFO("update result:%d", update_result);
    lwm2m_data_encode_int(update_result, data_array);

    if (update_result == UPDATE_RESULT_UPDATE_FAIL) {
        lwm2m_fota_set_update_result(fota_manager, UPDATE_RESULT_INIT);
    }
    return COAP_205_CONTENT;
}

static uint8_t fota_deliver_method_read(lwm2m_data_t* data_array)
{
    int method = 0;
    lwm2m_data_encode_int(method, data_array);
    return COAP_205_CONTENT;
}

static uint8_t prv_firmware_read_resources(int* num_data, lwm2m_data_t** data_array)
{
    if (*num_data == 0) {
        uint16_t resources[] = {
            RES_M_PACKAGE, RES_M_PACKAGE_URI, RES_M_STATE, RES_M_UPDATE_RESULT, RES_O_FIRMWARE_UPDATE_PROTOCOL_SUPPORT,
            RES_O_FIRMWARE_UPDATE_DELIVER_METHOD
        };
        size_t len = sizeof(resources) / sizeof(*resources);
        *data_array = lwm2m_data_new(len);
        if (*data_array == NULL) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        *num_data = len;
        for (uint32_t i = 0; i < len; ++i) {
            (*data_array)[i].id = resources[i];
        }
    }
    return COAP_205_CONTENT;
}

uint8_t prv_firmware_read(lwm2m_context_t * contextP, uint16_t instanceId, int * numDataP, lwm2m_data_t ** dataArrayP, lwm2m_object_t * objectP)
{
    uint8_t ret = COAP_404_NOT_FOUND;
    // this is a single instance object
    CHECK_ONLY(instanceId != 0, COAP_404_NOT_FOUND);

    // is the server asking for the full object ?
    ret = prv_firmware_read_resources(numDataP, dataArrayP);
    if (ret != COAP_205_CONTENT) {
        return ret;
    }

    uint32_t i = 0;
    do {
        switch ((*dataArrayP)[i].id) {
            case RES_M_PACKAGE:
                ret = COAP_405_METHOD_NOT_ALLOWED;
            case RES_M_PACKAGE_URI:
                ret = fota_uri_read(dataArrayP, i);
                break;
            case RES_M_UPDATE:
                ret = COAP_405_METHOD_NOT_ALLOWED;
                break;
            case RES_M_STATE: // firmware update state (int)
                ret = fota_state_read(contextP, (*dataArrayP) + i);
                break;
            case RES_M_UPDATE_RESULT:
                ret = fota_update_result_read(contextP, (*dataArrayP) + i);
                break;
            case RES_O_PKG_NAME:
            case RES_O_PKG_VERSION:
                ret = COAP_405_METHOD_NOT_ALLOWED;
                break;
            case RES_O_FIRMWARE_UPDATE_PROTOCOL_SUPPORT:
            case RES_O_FIRMWARE_UPDATE_DELIVER_METHOD:
                ret = fota_deliver_method_read((*dataArrayP) + i);
                break;
            default:
                ret = COAP_404_NOT_FOUND;
        }
        i++;
    } while (i < *numDataP && ret == COAP_205_CONTENT);
error:
    return ret;
}

static uint8_t fota_uri_write(lwm2m_context_t *contextP, lwm2m_data_t* data_array)
{
    int ret;
    if (data_array->value.asBuffer.buffer == NULL) {
        APP_LOG_INFO("type ERR %d", data_array->type);
    }

    APP_LOG_INFO("url %s", data_array->value.asBuffer.buffer);

    ret = lwm2m_fota_start_download(lwm2m_get_fota_manager(contextP),
        (const char*)(data_array->value.asBuffer.buffer), data_array->value.asBuffer.length);

    return (ret == LWM2M_FOTA_SUCCESS ? COAP_204_CHANGED : COAP_405_METHOD_NOT_ALLOWED);
}

static uint8_t fota_package_write(lwm2m_data_t* data_array, int item)
{
    if (data_array[item].type != LWM2M_TYPE_OPAQUE || data_array[item].value.asBuffer.buffer == NULL) {
        APP_LOG_INFO("type ERR %d", data_array[item].type);
        return COAP_400_BAD_REQUEST;
    }
    return COAP_204_CHANGED;
}

static uint8_t prv_firmware_write(lwm2m_context_t *contextP,
                                  uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP,
                                  lwm2m_write_type_t writeType)
{
    int i;
    uint8_t ret = COAP_404_NOT_FOUND;
    /* unused parameter */
    // All write types are treated the same here
    (void)writeType;

    CHECK_ONLY(instanceId != 0, COAP_404_NOT_FOUND);

    i = 0;
    do
    {
        /* No multiple instance resources */
        if (dataArray[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE)
        {
            ret = COAP_404_NOT_FOUND;
            continue;
        }

        switch (dataArray[i].id)
        {
        case RES_M_PACKAGE:
            ret = fota_package_write(dataArray, i);
            break;

        case RES_M_PACKAGE_URI:
            // URL for download the firmware
            ret = fota_uri_write(contextP, &dataArray[i]);
            break;

        default:
            ret = COAP_405_METHOD_NOT_ALLOWED;
        }

        i++;
    } while (i < numData && ret == COAP_204_CHANGED);
error:
    return ret;
}

static uint8_t fota_update_execute(lwm2m_context_t *contextP)
{
    int ret = lwm2m_fota_execute_update(lwm2m_get_fota_manager(contextP));
    if (ret == LWM2M_FOTA_SUCCESS) {
        return COAP_204_CHANGED;
    } else {
        // firmware update already running
        return COAP_400_BAD_REQUEST;
    }
}

static uint8_t prv_firmware_execute(lwm2m_context_t *contextP,
                                    uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP)
{
    uint8_t ret = COAP_404_NOT_FOUND;
    /* unused parameter */
    (void)contextP;
    // this is a single instance object
    CHECK_ONLY(instanceId != 0, COAP_404_NOT_FOUND);
    CHECK_ONLY(length != 0, COAP_400_BAD_REQUEST);

    // for execute callback, resId is always set.
    switch (resourceId)
    {
        case RES_M_UPDATE:
            return fota_update_execute(contextP);
        default:
            return COAP_405_METHOD_NOT_ALLOWED;
    }
error:
    return ret;
}

lwm2m_object_t* get_object_firmware()
{
    // The get_object_firmware function create the object itself and return a
    // pointer to the structure that represent it.
    lwm2m_object_t* firmware_obj = NULL;

    firmware_obj = (lwm2m_object_t*)lwm2m_malloc(sizeof(lwm2m_object_t));
    if (firmware_obj != NULL) {
        memset(firmware_obj, 0, sizeof(lwm2m_object_t));

        // It assigns its unique ID, The 5 is the standard ID for the optional
        // object "Object firmware".
        firmware_obj->objID = LWM2M_FIRMWARE_UPDATE_OBJECT_ID;

        // and its unique instance
        firmware_obj->instanceList = (lwm2m_list_t*)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (firmware_obj->instanceList != NULL) {
            memset(firmware_obj->instanceList, 0, sizeof(lwm2m_list_t));
        } else {
            lwm2m_free(firmware_obj);
            return NULL;
        }

        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made
         * by the server. In fact the library don't need to know the resources
         * of the object, only the server does.
         */
        firmware_obj->discoverFunc = prv_firmware_discover;
        firmware_obj->readFunc = prv_firmware_read;
        firmware_obj->writeFunc = prv_firmware_write;
        firmware_obj->executeFunc = prv_firmware_execute;

        // Also some user data can be stored in the object with a private
        // structure containing the needed variables
        firmware_obj->userData = NULL;
    }

    return firmware_obj;
}

void free_object_firmware(lwm2m_object_t* objectP)
{
    if (objectP == NULL) {
        return;
    }
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    if (NULL != objectP->instanceList)
    {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }
    lwm2m_free(objectP);
}
