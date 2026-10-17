

#include <time.h>
#include "esp_log.h"
#include "azimdb.h"
#include "display.h"


struct directionNames interestingDirections[] =
{
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0},
    {"-",        0}
};

static const char *TAG = "AZIMDB";
static uint8_t *chipid;

#define MAX_DIRECTIONS (sizeof(interestingDirections) / sizeof(struct directionNames))

static struct directionNames *find_freeSlot(int *index)
{
    *index = -1;

    for (int i=0; i < MAX_DIRECTIONS; i++)
    {
        if (interestingDirections[i].name[0] == '-')
        {
            *index = i;
            return &interestingDirections[i];
        }
    }
    return NULL;
}

static struct directionNames *find_byExactAzim(int azimuth, int *index)
{
    *index = -1;

    for (int i=0; i < MAX_DIRECTIONS;i++)
    {
        if (interestingDirections[i].azimuth == azimuth)
        {
            *index = i;
            return &interestingDirections[i];
        }
    }
    return NULL;
}

static void save_azim_value(int value, int index)
{
    char namestr[12];
    
    sprintf(namestr,"azvalue%02d", index);
    flash_write(setup_flash, namestr, value);
}

static void save_azim_name(char *name, int index)
{
    char namestr[12];
    
    sprintf(namestr,"azname%02d", index);
    flash_write_str(setup_flash, namestr, name);
}

static struct directionNames *find_by_name(char *name, int *index)
{
    *index = -1;
    for (int i=0; i < MAX_DIRECTIONS;i++)
    {
        if (!strcmp(interestingDirections[i].name, name))
        {
            *index = i;
            return &interestingDirections[i];
        }
    }
    return NULL;
}


static void azim_delindex(int index)
{
    char str[12];

    sprintf(str, "azname%02d", index);
    flash_erase_one(setup_flash, str);
    sprintf(str, "azvalue%02d", index);
    flash_erase_one(setup_flash, str);
    flash_commitchanges(setup_flash);
    interestingDirections[index].name[0]  ='-';
    interestingDirections[index].azimuth = 0;
}

bool azimdb_delname(char *name)
{
    int index;
    struct directionNames *rec;

    rec = find_by_name(name, &index);
    if (rec != NULL)
    {
        azim_delindex(index);
        display_printf(information,"%s deleted", name);
        return true;
    }
    return false;
}

static void read_all(void)
{
    char namestr[12];
    char valuestr[12];
    char *nameptr;
    char *def = "-";

    for (int i=0; i < MAX_DIRECTIONS;i++)
    {
        sprintf(namestr, "azname%02d", i);
        sprintf(valuestr, "azvalue%02d", i);
        nameptr = flash_read_str(setup_flash, namestr,  "-", 12);
        strcpy(interestingDirections[i].name, nameptr);
        if (nameptr != def)
        {
            free(nameptr);  // TODO: nameptr is a bit weird. rethink flash_ module.
            interestingDirections[i].azimuth = flash_read(setup_flash, valuestr, 0);
        }
    }
}

void azimdb_show(void)
{
    struct measurement meas;
    meas.id = ADB_REFRESHED;
    xQueueSend(evt_queue, &meas, 0);
}


char *azimdb_find_nearest(int azimuth, int beamwidth)
{
    int diff;
    int closest = 360;
    int closestIndex = -1;

    for (int i=0; i < MAX_DIRECTIONS ;i++)
    {
        if (interestingDirections[i].name[0] != '-')
        {
            diff = abs(interestingDirections[i].azimuth - azimuth);
            if (diff < closest)
            {
                closest = diff;
                closestIndex = i;
            }
        }
    }
    if ((closestIndex != -1) && (closest < (beamwidth / 2)))
    {
        return interestingDirections[closestIndex].name;
    }
    return "-";
}

struct directionNames *azimdb_find_named(char *name)
{
    int index;

    return find_by_name(name, &index);
}


bool azimdb_update(char *name, int azimuth)
{
    struct directionNames *rec;
    int index;

    // is it found by azimuth?
    // if so, and name differs, update the name
    rec = find_byExactAzim(azimuth, &index);
    if (rec != NULL)
    {
        ESP_LOGI(TAG,"found by azimuth");
        if (strcmp(rec->name, name))
        {
            ESP_LOGI(TAG,"updated name of azim %d to %s", azimuth, name);
            display_printf(information,"%s->%s", rec->name, name);
            strcpy(rec->name, name);
            save_azim_name(name, index);
            flash_commitchanges(setup_flash);
            return true;
        }
        else
        {
            ESP_LOGI(TAG,"Name is same, no update");
            return false;
        }
    }
    else
    {
        // otherwise, is it found by name
        // if so and azimuth differs, update azimuth
        rec = find_by_name(name, &index);
        if (rec != NULL)
        {
            ESP_LOGI(TAG,"found by name");
            if (rec->azimuth != azimuth)
            {
                ESP_LOGI(TAG,"updated %s azimuth from %d to %d", name, rec->azimuth, azimuth);
                display_printf(information,"%s:%d->%d", name, rec->azimuth, azimuth);
                rec->azimuth = azimuth;
                save_azim_value(azimuth, index);
                flash_commitchanges(setup_flash);
                return true;
            }
            else
            {
                ESP_LOGI(TAG,"azimuth not changed");
                return false;
            }
        }
    }
        // find a free slot.
    rec = find_freeSlot(&index);
    if (rec != NULL)
    {
        ESP_LOGI(TAG,"Inserting %s azimuth %d", name, azimuth);
        display_printf(information,"Adding %s", name);
        strcpy(rec->name, name);
        rec->azimuth = azimuth;
        save_azim_name(name, index);
        save_azim_value(azimuth, index);
        flash_commitchanges(setup_flash);
        return true;
    }
    else
    {
        ESP_LOGI(TAG,"directions table is full");
        display_printf(information,"Table is full");
    }
    return false;
}


void azimdb_publish(esp_mqtt_client_handle_t client, char *prefix, char *appname, time_t now)
{
    char topic[60];
    int retain=1;

    if (now < MIN_EPOCH)
    {
        retain = 0;
        now = 0;
    }
    sprintf(topic,"%s/%s/%x%x%x/setup/names", prefix, appname, chipid[3], chipid[4], chipid[5]);
    static char *datafmt = "{\"dev\":\"%x%x%x\",\"id\":\"nametable\",\"maxnames\":%d,\"ts\":%jd,\"names\":[ ";
    sprintf(jsondata, datafmt, chipid[3], chipid[4], chipid[5], MAX_DIRECTIONS, now);
    for (int i=0; i < MAX_DIRECTIONS;i++)
    {
        if (interestingDirections[i].name[0] != '-')
        {
            sprintf(&jsondata[strlen(jsondata)],"{\"name\":\"%s\",\"azimuth\":%d},", interestingDirections[i].name, interestingDirections[i].azimuth);
        }
    }
    jsondata[strlen(jsondata)-1]=0;
    strcat(jsondata,"]}");
    esp_mqtt_client_publish(client, topic, jsondata , 0, 0, retain);
}

bool azimdb_init(uint8_t *chip)
{
    chipid = chip;
    read_all();
    return true;
}
