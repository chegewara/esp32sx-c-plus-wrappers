#include <stdio.h>
#include <cstring>

#include "json.h"
#include "tools.h"

JSON *obj2;

static void createObject(int val)
{
    JSON obj;
    obj.add("int", val);
    obj.add("float", (float)val);
    char *pr = obj.print();
    // printf("obj: %s\n", pr);
    free(pr);

    JSON obj1(cJSON_String);
    char buf[10] = {};
    obj1.set(itoa(val, buf, 10));
    pr = obj1.print();
    // printf("array: %s\n", pr);
    free(pr);

    JSON arr(cJSON_Array);
    arr.addToArray(obj.get());
    arr.addToArray(obj1.get());
    pr = arr.print();
    // printf("array: %s\n", pr);
    free(pr);

    obj2->addArray(buf);
    pr = obj2->print();
    // obj2->minify(pr);
    // printf("obj2: %s\n", pr);
    free(pr);
}

static void destructor_test()
{
    obj2 = new JSON();
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < 100; j++)
        {
            createObject(i * 10 + j);
        }
        //
        printf("[ %d] free heap: %d, watermark: %d\n", i, Tools::internalFreeHeap(), Tools::watermark());
        vTaskDelay(5);
    }
    char* aa = obj2->print();
    printf("size: %d, => %d\n\n", strlen(aa), sizeof(cJSON));
    free(aa);
    delete (obj2);
    vTaskDelay(5);
    printf("[ %d] free heap: %d, watermark: %d\n\n\n", 0, Tools::internalFreeHeap(), Tools::watermark());
}

static void parse_test()
{
    const char *test = "{\
    \"debug\": \"on\",\
    \"width\": 640,\
    \"height\": 480,\
    \"window\": {\
        \"title\": \"Sample Konfabulator Widget\",\
        \"name\": \"main_window\"\
    },\
    \"image\": { \
        \"src\": \"Images/Sun.png\",\
        \"name\": \"sun1\",\
        \"hOffset\": 250,\
        \"vOffset\": 250,\
        \"alignment\": \"center\"\
    },\
    \"text\": {\
        \"data\": \"Click Here\",\
        \"size\": 36,\
        \"style\": \"bold\",\
        \"name\": \"text1\",\
        \"hOffset\": 250,\
        \"vOffset\": 100,\
        \"alignment\": \"center\",\
        \"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\"\
    }\
}";
    JSON parse(test);
    char *pr1 = parse.print();
    printf("obj2: %s\n\n\n", pr1);
    printf("test debug: %d, test width: %d, test height: %d\n\n\n", parse.exist("debug"), parse.exist("width"), parse.exist("height"));
    printf("debug: %s, width: %d, height: %d\n\n\n", parse.getString("debug"), parse.getInt("width"), parse.getInt("height"));
    free(pr1);
}

static void add_raw_test()
{
    JSON parse1;
    parse1.add("test", "{\"test1\":\"string\", \"test2\":123}", true);
    char *b = JSON(parse1.getObject("test")).print();
    JSON c(b);
    char* pr1 = parse1.print();
    // printf("obj2: %s\n\n\n", pr1);
    printf("test1: %d, test2: %d\n\n\n", c.exist("test1"), c.exist("test2"));
    // printf("%s\n\n", c.print());
    printf("test1: %s, test2: %d\n\n\n", c.getString("test1"), c.getInt("test2"));
    free(pr1);
    free(b);
}

extern "C" void app_main(void)
{
    while (1)
    {
        printf("[%d] free heap: %d, watermark: %d\n", -1, Tools::internalFreeHeap(), Tools::watermark());
        destructor_test();

        vTaskDelay(100);
        parse_test();
        vTaskDelay(100);

        add_raw_test();
        vTaskDelay(100);
    }
}
