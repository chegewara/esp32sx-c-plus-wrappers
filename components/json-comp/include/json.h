#pragma once 

#include "cJSON.h"


class JSON
{
private:
    cJSON *root;
    int type() { return root->type; }

public:
    JSON(const char* obj);
    JSON(cJSON* obj);
    JSON(uint8_t type = cJSON_Object);
    ~JSON();

public:
    static cJSON* createObject(cJSON* obj);
    cJSON* add(const char *key, bool val);
    cJSON* add(const char *key, int val);
    cJSON* add(const char *key, float val);
    cJSON* add(const char *key, double val);
    cJSON* add(const char *key, const char* val, bool raw = false);
    bool add(const char *key, cJSON *item);
    cJSON* addArray(const char *key);

    void set(char* val, cJSON* obj = NULL);
    void set(int val, cJSON* obj = NULL);
    void set(double val, cJSON* obj = NULL);
    void set(bool val, cJSON* obj = NULL);
    void set(float val, cJSON* obj = NULL);
    cJSON* get() { return root; }

    bool addToArray(cJSON* item);

    char* getString(const char* key = NULL);
    int getInt(const char* key = NULL);
    double getDouble(const char* key = NULL);
    cJSON* getObject(const char* key, bool sensitive = false);
    int getArraySize();
    cJSON* getArrayItem(int);
    bool exist(const char* key);
    char* print(cJSON* obj = NULL);
    static void minify(char* val);
    // bool compare();
};

