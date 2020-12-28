#include <cstring>

#include "sd-test.hpp"
#include "engine/api_private.hpp"

const int testSize = 0x10000;
const int numTests = 17;

uint32_t writeTime = 0;
size_t numFiles = 0;
uint32_t results[numTests];
int curTest = 0;

uint8_t testData[testSize];

std::string error;

uint32_t getElapsedTime(uint32_t start, uint32_t end)
{
    if(end >= start)
        return end - start;
    else
        return (blit::api.get_max_us_timer() - start) + end;
}

void init()
{
    blit::set_screen_mode(blit::ScreenMode::hires);

    for(auto &b : testData)
        b = blit::random();

    auto start = blit::api.get_us_timer();
    blit::File f("sdtest.dat", blit::OpenMode::write);

    auto written = f.write(0, sizeof(testData), reinterpret_cast<const char *>(testData));
    f.close();

    if(written != testSize)
        error = "Failed to create test data\n";
    else
        writeTime = getElapsedTime(start, blit::api.get_us_timer());

    numFiles = blit::list_files("").size();
}

void render(uint32_t time_ms)
{
    blit::screen.pen = blit::Pen(20, 30, 40);
    blit::screen.clear();

    blit::screen.pen = blit::Pen(255, 255, 255);

    char buf[100];
    int y = 0;

    snprintf(buf, 100, "Wrote test data in %ius\n", writeTime);
    blit::screen.text(buf, blit::minimal_font, blit::Point(0, y));
    y += 10;

    snprintf(buf, 100, "Card has %i files at root...\n", static_cast<int>(numFiles));
    blit::screen.text(buf, blit::minimal_font, blit::Point(0, y));
    y+= 15;

    for(int i = 0; i < curTest; i++)
    {
        float speed = static_cast<float>(testSize) / (static_cast<float>(results[i]) / 1000000.0f);
        const char *unit = "B";

        if(speed >= 1000.0f)
        {
            speed /= 1000.0f;
            unit = "kB";
        }

        if(speed >= 1000.0f)
        {
            speed /= 1000.0f;
            unit = "MB";
        }

        snprintf(buf, 100, "read %5i bytes x%5i in %5ius %3.3f%s/s\n", 1 << i, testSize >> i, results[i], (double)speed, unit);
        blit::screen.text(buf, blit::minimal_font, blit::Point(0, y), false);

        y += 10;
    }

    if(!error.empty())
    {
        y += 5;
        blit::screen.pen = blit::Pen(0xFF, 0, 0);
        blit::screen.text(error, blit::minimal_font, blit::Point(0, y));
    }
}

void update(uint32_t time_ms)
{
    if(curTest < numTests && error.empty())
    {
        blit::File f("sdtest.dat");
        char buf[testSize];

        int size = 1 << curTest;
        int count = testSize >> curTest;

        uint32_t start = blit::api.get_us_timer();
        for(int j = 0; j < count; j++)
        {
            if(f.read(j * size, size, buf + j * size) != size)
            {
                error = "Read failed!";
                break;
            }
        }

        results[curTest++] = getElapsedTime(start, blit::api.get_us_timer());

        if(memcmp(buf, testData, testSize) != 0)
            error = "Data mismatch!";
    }
}
