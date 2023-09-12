#ifndef _XLCD_FIRMWARE
#define _XLCD_FIRMWARE

#include <Arduino.h>
#include <Update.h>
#include <SD.h>
#include <stdio.h>
#include <semver.h>

bool xtouch_firmware_hasFirmwareUpdate = false;

byte xtouch_firmware_semverNeedsUpdate(String compare)
{
    semver_t current_version = {};
    semver_t compare_version = {};

    if (semver_parse(XTOUCH_FIRMWARE_VERSION, &current_version) || semver_parse(compare.c_str(), &compare_version))
    {
        return -1;
    }

    int resolution = semver_compare(compare_version, current_version);

    // Free allocated memory when we're done
    semver_free(&current_version);
    semver_free(&compare_version);

    if (resolution == 0 || resolution == -1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void xtouch_firmware_onProgress(size_t currSize, size_t totalSize)
{
    int16_t progress = (currSize * 100) / totalSize;
    lv_label_set_text_fmt(introScreenCaption, LV_SYMBOL_CHARGE " Updating %d%%", progress);
    lv_timer_handler();
    lv_task_handler();
}

void xtouch_firmware_checkOnlineFirmwareUpdate(void)
{
    if (!xTouchConfig.xTouchOTAEnabled)
    {
        return;
    }

    bool hasOTAConfigFile = downloadFileToSDCard(xtouch_paths_firmware_ota_file, xtouch_paths_firmware_ota_json);

    if (hasOTAConfigFile)
    {

        DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_firmware_ota_json);

        String version = doc["version"];
        String url = doc["url"];

        if (xtouch_firmware_semverNeedsUpdate(version) == 1)
        {

            xtouch_firmware_hasFirmwareUpdate = downloadFileToSDCard(url.c_str(), xtouch_paths_firmware_ota_fw, [](int progress)
                                                                     {
        lv_label_set_text_fmt(introScreenCaption, LV_SYMBOL_CHARGE " Downloading Update %d%%", progress);
        lv_timer_handler();
        lv_task_handler(); });

            if (xtouch_firmware_hasFirmwareUpdate)
            {

                lv_label_set_text(introScreenCaption, LV_SYMBOL_OK " Update downloaded");
                lv_timer_handler();
                lv_task_handler();
                delay(3000);
                ESP.restart();
            }
            else
            {
                lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " Failed to download update");
                lv_timer_handler();
                lv_task_handler();
                delay(3000);
            }
        }
        xtouch_filesystem_deleteFile(SD, xtouch_paths_firmware_ota_json);
    }
    else
    {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " Failed to download update");
        lv_timer_handler();
        lv_task_handler();
        delay(3000);
    }
}

void xtouch_firmware_checkFirmwareUpdate(void)
{

    // Verifica si el archivo de actualización existe en la tarjeta SD
    if (xtouch_filesystem_exist(SD, xtouch_paths_firmware_ota_fw))
    {
        if (xtouch_filesystem_exist(SD, xtouch_paths_firmware_ota_fw))
        {
            File firmware = xtouch_filesystem_open(SD, xtouch_paths_firmware_ota_fw);
            Update.onProgress(xtouch_firmware_onProgress);
            Update.begin(firmware.size(), U_FLASH);
            Update.writeStream(firmware);
            bool updateSucceeded = Update.end();
            firmware.close();

            if (updateSucceeded)
            {
                lv_label_set_text(introScreenCaption, LV_SYMBOL_OK " Update finished");
                lv_timer_handler();
                lv_task_handler();
            }
            else
            {
                lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " Update error");
                lv_timer_handler();
                lv_task_handler();
                delay(3000);
                lv_label_set_text(introScreenCaption, LV_SYMBOL_TRASH " Deleting firmware file");
                lv_timer_handler();
                lv_task_handler();
            }

            xtouch_filesystem_deleteFile(SD, xtouch_paths_firmware_ota_fw);

            delay(2000);
            ESP.restart();
        }
    }
}
#endif