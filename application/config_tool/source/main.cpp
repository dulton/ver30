#include "application.h"

// Process entry, 'main' function
int main(int argc, const char * argv [])
{
    // Define the new application instance
    Application App(argc, argv);

    // Start this application
    GMI_RESULT  RetVal = App.Start();

    // Return this process
    return RetVal == GMI_SUCCESS ? 0 : 1;
}

