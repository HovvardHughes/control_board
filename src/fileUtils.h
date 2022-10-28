#include <SPIFFS.h>

void writeFile(fs::FS &fs, const char *path, const char *message)
{
    File file = fs.open(path, FILE_WRITE);
    file.print(message);
}

String readFile(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
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