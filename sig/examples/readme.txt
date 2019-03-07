
Each folder here contains source files of example applications
which are compiled from the main sig project files.

File sigapp.7z contains a project that can be used
as a starting point to build your own sig application
outside the SIG folder. Follow these steps:

1. Unzip sigapp.7z in the same folder as your sig installation.
   Sigapp uses relative paths to find SIG's libraries and it
   assumes folders sig/ and sigapp/ are on the same directory.

2. You can now open sigapp/vs2017/sigapp.sln with Visual Studio
   to work on your new project.

3. Rename your sigapp folder to your project's name.

File sigmynode.7z also contains a project ready to be compiled
outside the SIG folder - just follow the same steps given above.
Project sigmynode.7z can be used to test/build a new node and 
node renderer. 
