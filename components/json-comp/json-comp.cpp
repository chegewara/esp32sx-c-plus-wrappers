#include <stdio.h>
#include "cJSON.h"

#include "json.h"

JSON::JSON(const char *obj)
{
    root = cJSON_Parse(obj);
}

JSON::JSON(cJSON *obj)
{
    root = cJSON_Duplicate(obj, true);
}

JSON::JSON(uint8_t type)
{
    switch (type)
    {
    case cJSON_False:
        root = cJSON_CreateFalse();
        break;
    case cJSON_True:
        root = cJSON_CreateTrue();
        break;
    case cJSON_NULL:
        root = cJSON_CreateNull();
        break;
    case cJSON_Number:
        root = cJSON_CreateNumber(0);
        break;
    case cJSON_String:
        root = cJSON_CreateString("");
        break;
    case cJSON_Array:
        root = cJSON_CreateArray();
        break;
    case cJSON_Object:
        root = cJSON_CreateObject();
        break;
    case cJSON_Raw:
        root = cJSON_CreateRaw("");
        break;

    default:
        root = cJSON_CreateObject();
        break;
    }
}

JSON::~JSON()
{
    cJSON_Delete(root);
}

cJSON *JSON::add(const char *key, bool val)
{
    return cJSON_AddBoolToObject(root, key, val);
}

cJSON *JSON::add(const char *key, int val)
{
    return cJSON_AddNumberToObject(root, key, val);
}

cJSON *JSON::add(const char *key, float val)
{
    return cJSON_AddNumberToObject(root, key, val);
}

cJSON *JSON::add(const char *key, double val)
{
    return cJSON_AddNumberToObject(root, key, val);
}

cJSON *JSON::add(const char *key, const char *val, bool raw)
{
    if (raw)
        return cJSON_AddRawToObject(root, key, val);

    return cJSON_AddStringToObject(root, key, val);
}

bool JSON::add(const char *key, cJSON *item)
{
    return cJSON_AddItemReferenceToObject(root, key, item);
}

cJSON *JSON::addArray(const char *key)
{
    return cJSON_AddArrayToObject(root, key);
}

bool JSON::addToArray(cJSON *obj)
{
    return cJSON_AddItemReferenceToArray(root, obj);
}

cJSON *JSON::getObject(const char *key, bool sensitive)
{
    if (sensitive)
        return cJSON_GetObjectItemCaseSensitive(root, key);
    return cJSON_GetObjectItem(root, key);
}

bool JSON::exist(const char *key)
{
    return cJSON_HasObjectItem(root, key);
}

void JSON::set(char *val, cJSON *obj)
{
    if (NULL == obj)
    {
        obj = root;
    }
    cJSON_SetValuestring(obj, val);
}

void JSON::set(int val, cJSON *obj)
{
    if (NULL == obj)
    {
        obj = root;
    }
    cJSON_SetIntValue(obj, val);
}

void JSON::set(double val, cJSON *obj)
{
    if (NULL == obj)
    {
        obj = root;
    }
    cJSON_SetNumberValue(obj, val);
}

void JSON::set(bool val, cJSON *obj)
{
    if (NULL == obj)
    {
        obj = root;
    }
    cJSON_SetIntValue(obj, val);
}

void JSON::set(float val, cJSON *obj)
{
    if (NULL == obj)
    {
        obj = root;
    }
    cJSON_SetNumberValue(obj, val);
}

char *JSON::print(cJSON *obj)
{
    if (NULL == obj)
    {
        obj = root;
    }
    return cJSON_Print(obj);
}

void JSON::minify(char *val)
{
    cJSON_Minify(val);
}

char *JSON::getString(const char* key)
{
    if(!key) return cJSON_GetStringValue(root);

    return cJSON_GetStringValue(cJSON_GetObjectItem(root, key));
}

int JSON::getInt(const char* key)
{
    if (cJSON_IsNumber(root))
    {
        return root->valueint;
    }
    
    return cJSON_GetObjectItem(root, key)->valueint;
}

double JSON::getDouble(const char* key)
{
    if(!key) return cJSON_GetNumberValue(root);

    return cJSON_GetNumberValue(cJSON_GetObjectItem(root, key));
}

cJSON* JSON::createObject(cJSON* obj)
{
    return cJSON_Duplicate(obj, true);
}
