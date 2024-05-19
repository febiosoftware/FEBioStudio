#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#ifdef WIN32
#include <Windows.h>
#include <WinBase.h>
#endif

int main(int argc, char* argv[])
{
    if(argc < 3) return -1;

    int start = 2;
    bool dev = false;

    if(strcmp(argv[2], "-d") == 0)
    {
        start++;
        dev = true;
    }

    if((argc - start) % 2 != 0) return -1;

    std::ofstream myfile;
    myfile.open ("mvUtil.log");

    myfile << "Args:" << std::endl;

    for(int index = 0; index < argc; index++)
    {
        myfile << argv[index] << std::endl;
    }

    myfile << std::endl;

	for (int index = start; index < argc; index += 2)
	{

// Windows won't allow a file to be overwritten by std::rename, so we 
// delete it first. We also pause and loop so that we can be sure that 
// the lock on the auto-updater executable is gone 
#ifdef WIN32
        int n = 0;
		while (std::remove(argv[index + 1]) != 0)
		{
            // If the file just doesn't exist, break
            if(errno == ENOENT) break;

            myfile << "Failed to delete " << argv[index + 1] << ". Error: " << errno << std::endl;

			_sleep(500);
			n++;
			if (n > 10) break;
		}
#endif

		// rename the file
		std::rename(argv[index], argv[index + 1]);
        
#ifndef WIN32
        chmod(argv[1], S_IRWXU|S_IXGRP|S_IXOTH);
#endif
    }

    myfile.close();

    char command[1000];

    if(dev)
    {
        sprintf(command, "\"%s\" --noUpdaterCheck --devChannel", argv[1]);
    }
    else
    {
        sprintf(command, "\"%s\" --noUpdaterCheck", argv[1]);
    }

    std::system(command);
}
