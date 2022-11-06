#include <SPIFFS.h>

void writeFile(const char *path, const char *data)
{
    File file = SPIFFS.open(path, FILE_WRITE);
    file.print(data);
}

String readFile(const char *path)
{
    File file = SPIFFS.open(path);
    if (!file || file.isDirectory())
        return String();

    String fileContent;
    while (file.available())
    {
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

void initSPIFFS()
{
    SPIFFS.begin(true);
}