# tasklist_exec


This program simulates the actions of the Windows Executable tasklist.exe. It enumerates all running processes on the system and provides information related to those processes. The information displayed can be customized based on optional flags provided by the user (described below).

How this program operates:
- The program takes a snapshot of all the running process using the function CreateToolhelp32Snapshot. This creates a structure holding all running processes on the - system
- The program steps through all the process items in the structure (using Process32First and Process32Next) and extracts the process information from each process to display to the user
- Much of the process information is stored in the PROCESSENTRY32 object; however, any additional process information that is displayed for the user is extracted using additional Windows32 API calls.

Optional Flags:

**/V**
(Verbose): Provides additional information about the process that is not normally included in the standard display

**/SVC** 
(Services): If the process is running as a service, shows the service running within the process 

**/S computer-name**
: displays the processes running on the remote computer specified in computer-name

**/U username**
: runs the process with the permissions of the user specified in username. This can only be run if /S is also specified
The user will also be required to input a password for the specified user when prompted (if the user does not have a password, it can be left blank)
