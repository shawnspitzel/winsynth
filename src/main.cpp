#include "App.h"

App* g_AppInstance = nullptr;

int main()
{
    App app;
    g_AppInstance = &app;

    if (!app.Initialize())
    {
        return 1;
    }

    app.Run();
    app.Shutdown();

    g_AppInstance = nullptr;

    return 0;
}